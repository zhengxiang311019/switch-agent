#include "tcp_analysis.h"
#include <string.h>
#include <limits.h>

#define TCP_SYN 0x02U
#define TCP_FIN 0x01U
#define TCP_RST 0x04U
#define TCP_ACK 0x10U
static uint32_t le32t(const uint8_t *p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static int same(const ot_tcp_flow_entry_t *e,const ot_packet_view_t *p)
{ return e->used&&e->ip_version==p->ip_version&&e->src_port==p->src_port&&e->dst_port==p->dst_port&&e->vlan_count==p->vlan_count&&memcmp(e->src_ip,p->src_ip,16)==0&&memcmp(e->dst_ip,p->dst_ip,16)==0&&memcmp(e->vlan_id,p->vlan_id,sizeof(uint16_t)*p->vlan_count)==0; }
static int reverse(const ot_tcp_flow_entry_t *e,const ot_packet_view_t *p)
{ return e->used&&e->ip_version==p->ip_version&&e->src_port==p->dst_port&&e->dst_port==p->src_port&&e->vlan_count==p->vlan_count&&memcmp(e->src_ip,p->dst_ip,16)==0&&memcmp(e->dst_ip,p->src_ip,16)==0&&memcmp(e->vlan_id,p->vlan_id,sizeof(uint16_t)*p->vlan_count)==0; }
static ot_tcp_flow_entry_t *get(ot_tcp_tracker_t *t,const ot_packet_view_t *p,uint64_t now)
{
 for(size_t i=0;i<t->count;i++)if(same(&t->flows[i],p))return &t->flows[i];
 ot_tcp_flow_entry_t *e=NULL;
 if(t->count>=OT_TCP_FLOW_CAPACITY){size_t oldest=SIZE_MAX;for(size_t i=0;i<t->count;i++){ot_tcp_flow_entry_t *candidate=&t->flows[i];if(candidate->stats.rst_seen||(now&&candidate->stats.last_seen_ns&&now>candidate->stats.last_seen_ns&&now-candidate->stats.last_seen_ns>120000000000ULL)){if(oldest==SIZE_MAX||candidate->stats.last_seen_ns<t->flows[oldest].stats.last_seen_ns)oldest=i;}}if(oldest==SIZE_MAX){t->flow_capacity_drops++;return NULL;}e=&t->flows[oldest];}
 else e=&t->flows[t->count++];
 memset(e,0,sizeof(*e));e->used=1;e->ip_version=p->ip_version;e->src_port=p->src_port;e->dst_port=p->dst_port;e->vlan_count=p->vlan_count;memcpy(e->src_ip,p->src_ip,16);memcpy(e->dst_ip,p->dst_ip,16);memcpy(e->vlan_id,p->vlan_id,sizeof(uint16_t)*p->vlan_count);return e;
}
static void ack_reverse(ot_tcp_tracker_t *t,const ot_packet_view_t *p,uint64_t now)
{
 if(!(p->tcp_flags&TCP_ACK))return;
 for(size_t i=0;i<t->count;i++)if(reverse(&t->flows[i],p)){ot_tcp_flow_entry_t *e=&t->flows[i];ot_tcp_flow_stats_t *s=&e->stats;
  if(s->last_acknowledgment==p->tcp_acknowledgment && p->tcp_acknowledgment!=0U && s->bytes_in_flight)s->duplicate_acks++;
  if((int32_t)(p->tcp_acknowledgment-s->last_acknowledgment)>0){s->acked_bytes+=(uint32_t)(p->tcp_acknowledgment-s->last_acknowledgment);s->last_acknowledgment=p->tcp_acknowledgment;}
  for(size_t j=0;j<OT_TCP_OUTSTANDING_CAPACITY;j++){ot_tcp_outstanding_t *o=&e->outstanding[j];if(o->used&&(int32_t)(p->tcp_acknowledgment-o->end_sequence)>=0){if(!o->retransmitted&&now&&o->sent_ns&&now>=o->sent_ns){s->ack_rtt_count++;s->ack_rtt_sum_ns+=now-o->sent_ns;}o->used=0;}}
  uint32_t inflight=0;for(size_t j=0;j<OT_TCP_OUTSTANDING_CAPACITY;j++)if(e->outstanding[j].used)inflight+=e->outstanding[j].end_sequence-e->outstanding[j].start_sequence;s->bytes_in_flight=inflight;break;
 }
}
static size_t pdu_size(const ot_tcp_flow_entry_t *e,const uint8_t *b,size_t n)
{
 uint16_t port=e->src_port==44818U||e->dst_port==44818U?44818U:(e->src_port==502U||e->dst_port==502U?502U:(e->src_port==4840U||e->dst_port==4840U?4840U:(e->src_port==8883U||e->dst_port==8883U?8883U:(e->src_port==1883U||e->dst_port==1883U?1883U:0U))));
 if(port==44818U){if(n<24U)return 0;size_t z=24U+(size_t)(b[2]|((uint16_t)b[3]<<8));return z<=OT_TCP_MAX_APPLICATION_PDU?z:SIZE_MAX;}
 if(port==502U){if(n<6U)return 0;size_t z=6U+((size_t)b[4]<<8)+b[5];return z<=260U?z:SIZE_MAX;}
 if(port==4840U){if(n<8U)return 0;size_t z=le32t(b+4U);return z>=8U&&z<=OT_TCP_MAX_APPLICATION_PDU?z:SIZE_MAX;}
 if(port==8883U){if(n<5U)return 0;if(b[0]<20U||b[0]>23U||b[1]!=3U)return SIZE_MAX;size_t z=5U+((size_t)b[3]<<8)+b[4];return z<=OT_TCP_MAX_APPLICATION_PDU?z:SIZE_MAX;}
 if(port==1883U){if(n<2U)return 0;uint32_t rem=0;unsigned shift=0U,i=1U;uint8_t d=0;do{if(i>=n)return 0;if(i>4U)return SIZE_MAX;d=b[i++];rem+=(uint32_t)(d&127U)<<shift;shift+=7U;}while(d&128U);size_t head=i;return (size_t)rem<=OT_TCP_MAX_APPLICATION_PDU&&head+(size_t)rem<=OT_TCP_MAX_APPLICATION_PDU?head+(size_t)rem:SIZE_MAX;}
 return SIZE_MAX;
}
static ot_decode_result_t decode_pdu(const ot_tcp_flow_entry_t *e,const uint8_t *b,size_t n)
{
 if(e->src_port==44818U||e->dst_port==44818U)return ot_decode_enip(b,n,44818U);
 if(e->src_port==502U||e->dst_port==502U)return ot_decode_modbus_tcp(b,n);
 if(e->src_port==4840U||e->dst_port==4840U)return ot_decode_opcua(b,n);
 if(e->src_port==8883U||e->dst_port==8883U)return ot_decode_mqtt(b,n,1);
 if(e->src_port==1883U||e->dst_port==1883U)return ot_decode_mqtt(b,n,0);
 ot_decode_result_t r;memset(&r,0,sizeof(r));r.state=OT_PARSE_UNPARSEABLE;return r;
}
static void parse_application(ot_tcp_flow_entry_t *e)
{
 while(e->application_length){size_t z=pdu_size(e,e->application_bytes,e->application_length);if(z==0U)return;if(z==SIZE_MAX){e->stats.application_parse_state=OT_PARSE_UNPARSEABLE;e->application_length=0;memset(e->application_bytes,0,sizeof(e->application_bytes));return;}if(z>e->application_length)return;
  ot_decode_result_t r=decode_pdu(e,e->application_bytes,z);e->stats.application_protocol=(uint8_t)r.protocol;e->stats.application_business_class=(uint8_t)r.business_class;e->stats.application_parse_state=(uint8_t)r.state;e->stats.application_pdu_count++;
  size_t old_length=e->application_length;memmove(e->application_bytes,e->application_bytes+z,old_length-z);e->application_length=old_length-z;memset(e->application_bytes+e->application_length,0,z);
 }
}
static void reassemble_payload(ot_tcp_flow_entry_t *e,const ot_packet_view_t *p,const uint8_t *frame,uint32_t seq,uint64_t ts,ot_capture_coverage_t coverage)
{
 if(!p->payload_length)return;
 ot_tcp_flow_stats_t *s=&e->stats;uint32_t expected=s->next_expected_sequence;int32_t delta=(int32_t)(seq-expected);size_t skip=0U;
 if(delta<0){skip=(size_t)(-(int64_t)delta);if(skip>=p->payload_length){s->retransmission_candidates++;return;}s->retransmission_candidates++;}
 else if(delta>0){if(!s->gap_active){s->sequence_gaps++;if(coverage==OT_CAPTURE_COVERAGE_BIDIRECTIONAL)s->confirmed_capture_gaps++;else s->unconfirmed_gaps++;s->gap_active=1;s->gap_started_ns=ts;s->gap_timeout_reported=0;}s->out_of_order_candidates++;}
 size_t base=delta>0?(size_t)delta:0U;size_t available=p->payload_length-skip;if(base>=sizeof(e->stream_bytes)||available>sizeof(e->stream_bytes)-base){s->reassembly_overflows++;return;}
 for(size_t i=0;i<available;i++){size_t at=base+i;if(e->stream_present[at]){s->retransmission_candidates++;continue;}e->stream_bytes[at]=frame[p->payload_offset+skip+i];e->stream_present[at]=1;}
 size_t contiguous=0U;while(contiguous<sizeof(e->stream_present)&&e->stream_present[contiguous])contiguous++;
 if(contiguous){size_t room=sizeof(e->application_bytes)-e->application_length;size_t move=contiguous<room?contiguous:room;if(move)memcpy(e->application_bytes+e->application_length,e->stream_bytes,move);e->application_length+=move;if(move<contiguous){s->reassembly_overflows++;e->application_length=0;memset(e->application_bytes,0,sizeof(e->application_bytes));}
  memmove(e->stream_bytes,e->stream_bytes+contiguous,sizeof(e->stream_bytes)-contiguous);memset(e->stream_bytes+sizeof(e->stream_bytes)-contiguous,0,contiguous);
  memmove(e->stream_present,e->stream_present+contiguous,sizeof(e->stream_present)-contiguous);memset(e->stream_present+sizeof(e->stream_present)-contiguous,0,contiguous);
  s->next_expected_sequence+=(uint32_t)contiguous;if(s->gap_active&&(int32_t)(s->next_expected_sequence-s->highest_sequence)>=0){s->gap_active=0;s->gap_timeout_reported=0;s->resolved_gaps++;}parse_application(e);
 }
 (void)ts;
}
void ot_tcp_tracker_init(ot_tcp_tracker_t *t,uint64_t rto){if(t){memset(t,0,sizeof(*t));t->rto_ns=rto?rto:1000000000ULL;}}
void ot_tcp_tracker_expire(ot_tcp_tracker_t *t,uint64_t now)
{ if(!t||!now)return;for(size_t i=0;i<t->count;i++){ot_tcp_flow_entry_t *e=&t->flows[i];ot_tcp_flow_stats_t *s=&e->stats;if(s->gap_active&&!s->gap_timeout_reported&&s->gap_started_ns&&now>=s->gap_started_ns&&now-s->gap_started_ns>=t->rto_ns){s->gap_timeout_candidates++;s->gap_timeout_reported=1;}for(size_t j=0;j<OT_TCP_OUTSTANDING_CAPACITY;j++){ot_tcp_outstanding_t *o=&e->outstanding[j];if(o->used&&o->sent_ns&&now>=o->sent_ns&&now-o->sent_ns>=t->rto_ns){o->used=0;s->ack_timeout_candidates++;}}uint32_t inflight=0;for(size_t j=0;j<OT_TCP_OUTSTANDING_CAPACITY;j++)if(e->outstanding[j].used)inflight+=e->outstanding[j].end_sequence-e->outstanding[j].start_sequence;s->bytes_in_flight=inflight;} }
int ot_tcp_tracker_ingest(ot_tcp_tracker_t *t,const ot_packet_view_t *p,const uint8_t *frame,uint64_t ts,ot_capture_coverage_t coverage)
{
 if(!t||!p||!frame||p->transport_protocol!=6U)return 0;
 ot_tcp_flow_entry_t *e=get(t,p,ts);if(!e)return 0;ot_tcp_flow_stats_t *s=&e->stats;
 if((p->tcp_flags&TCP_SYN)&&s->syn_seen&&s->initial_sequence!=p->tcp_sequence){uint32_t generation=s->connection_generation+1U;memset(s,0,sizeof(*s));s->connection_generation=generation;memset(e->outstanding,0,sizeof(e->outstanding));memset(e->stream_bytes,0,sizeof(e->stream_bytes));memset(e->stream_present,0,sizeof(e->stream_present));memset(e->application_bytes,0,sizeof(e->application_bytes));e->application_length=0;}
 ack_reverse(t,p,ts);s->packets++;s->payload_bytes+=p->payload_length;s->last_seen_ns=ts;s->coverage=(uint8_t)coverage;s->last_window=p->tcp_window;if(!p->tcp_options_valid)s->tcp_option_errors++;s->sack_blocks_seen+=p->tcp_sack_blocks;if(p->tcp_timestamp_value)s->timestamp_options_seen++;
 uint32_t seq=p->tcp_sequence+((p->tcp_flags&TCP_SYN)?1U:0U);uint32_t span=(uint32_t)p->payload_length+((p->tcp_flags&TCP_FIN)?1U:0U);
 if(p->tcp_flags&TCP_SYN){s->syn_seen=1;s->initial_sequence=p->tcp_sequence;if(!s->sequence_initialized){s->sequence_initialized=1;s->next_expected_sequence=seq;s->highest_sequence=seq;}}
 if(span){uint32_t end=seq+span;
  if(!s->sequence_initialized){s->sequence_initialized=1;s->next_expected_sequence=seq;s->highest_sequence=seq;}
  if((int32_t)(end-s->highest_sequence)>0)s->highest_sequence=end;
  if(p->payload_length)reassemble_payload(e,p,frame,seq,ts,coverage);
  if((p->tcp_flags&TCP_FIN)&&(int32_t)(seq+(uint32_t)p->payload_length-s->next_expected_sequence)==0){s->next_expected_sequence++;if(s->gap_active&&(int32_t)(s->next_expected_sequence-s->highest_sequence)>=0){s->gap_active=0;s->gap_timeout_reported=0;s->resolved_gaps++;}}
  ot_tcp_outstanding_t *slot=NULL;for(size_t i=0;i<OT_TCP_OUTSTANDING_CAPACITY;i++){if(e->outstanding[i].used&&e->outstanding[i].end_sequence==end){e->outstanding[i].retransmitted=1;slot=NULL;break;}if(!e->outstanding[i].used&&!slot)slot=&e->outstanding[i];}
  int overlaps=0;for(size_t i=0;i<OT_TCP_OUTSTANDING_CAPACITY;i++){ot_tcp_outstanding_t *o=&e->outstanding[i];if(o->used&&(int32_t)(end-o->start_sequence)>0&&(int32_t)(o->end_sequence-seq)>0){o->retransmitted=1;overlaps=1;}}
  if(slot&&!overlaps&&ts){slot->used=1;slot->start_sequence=seq;slot->end_sequence=end;slot->sent_ns=ts;slot->retransmitted=0;uint32_t inflight=0;for(size_t i=0;i<OT_TCP_OUTSTANDING_CAPACITY;i++)if(e->outstanding[i].used)inflight+=e->outstanding[i].end_sequence-e->outstanding[i].start_sequence;s->bytes_in_flight=inflight;}
 }
 if(p->tcp_flags&TCP_FIN)s->fin_seen=1;
 if(p->tcp_flags&TCP_RST)s->rst_seen=1;
 ot_tcp_tracker_expire(t,ts);return 1;
}
const ot_tcp_flow_entry_t *ot_tcp_tracker_find(const ot_tcp_tracker_t *t,const ot_packet_view_t *p)
{ if(!t||!p)return NULL;for(size_t i=0;i<t->count;i++)if(same(&t->flows[i],p))return &t->flows[i];return NULL; }
