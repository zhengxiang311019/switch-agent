#include "engine.h"
#include "../protocols/protocols.h"
#include <string.h>

void ot_analyzer_engine_init(ot_analyzer_engine_t *e,uint64_t width)
{if(e){memset(e,0,sizeof(*e));ot_flow_catalog_init(&e->flows);ot_traffic_aggregator_init(&e->traffic,width);ot_observation_health_init(&e->health);ot_tcp_tracker_init(&e->tcp,1000000000ULL);e->capture_coverage=OT_CAPTURE_COVERAGE_UNKNOWN;}}
void ot_analyzer_engine_set_capture_coverage(ot_analyzer_engine_t *e,ot_capture_coverage_t c){if(e)e->capture_coverage=c;}
int ot_analyzer_ingest(ot_analyzer_engine_t *e,uint32_t scope,const uint8_t *f,size_t n,uint64_t ts)
{
 if(!e||!f)return 0;
 if(n<14U){ot_traffic_aggregator_add(&e->traffic,scope,ts,n,0);e->total_packets++;e->total_bytes+=n;e->health.input_packets++;e->health.truncated_packets++;e->health.parser_rejects++;return 1;}
 ot_packet_view_t v;memset(&v,0,sizeof(v));int view_ok=ot_packet_view_decode(f,n,&v);if(!view_ok){memcpy(v.src_mac,f+6,6);memcpy(v.dst_mac,f,6);}
 ot_decode_result_t d=ot_protocol_decode(f,n);
 ot_flow_key_t k;memset(&k,0,sizeof(k));memcpy(k.src_mac,v.src_mac,6);memcpy(k.dst_mac,v.dst_mac,6);k.ip_version=v.ip_version;memcpy(k.src_ip,v.src_ip,16);memcpy(k.dst_ip,v.dst_ip,16);k.vlan_count=v.vlan_count;memcpy(k.vlan_id,v.vlan_id,sizeof(k.vlan_id));k.protocol_id=(uint16_t)d.protocol;k.ip_protocol=v.transport_protocol;k.src_port=v.src_port;k.dst_port=v.dst_port;
 uint64_t pdu_before=0;
 if(view_ok&&v.transport_protocol==6U&&!v.fragmented){const ot_tcp_flow_entry_t *old=ot_tcp_tracker_find(&e->tcp,&v);if(old)pdu_before=old->stats.application_pdu_count;ot_tcp_tracker_ingest(&e->tcp,&v,f,ts,e->capture_coverage);const ot_tcp_flow_entry_t *flow=ot_tcp_tracker_find(&e->tcp,&v);if(flow&&flow->stats.application_pdu_count>pdu_before&&flow->stats.application_protocol!=OT_PROTO_UNKNOWN){d.protocol=(ot_protocol_t)flow->stats.application_protocol;d.business_class=(ot_business_class_t)flow->stats.application_business_class;d.state=(ot_parse_state_t)flow->stats.application_parse_state;}}
 k.protocol_id=(uint16_t)d.protocol;
 if(d.protocol!=OT_PROTO_UNKNOWN){if(!ot_flow_catalog_observe(&e->flows,&k,&d,ts,n))return 0;if(!ot_traffic_aggregator_add(&e->traffic,scope,ts,n,1))return 0;}
 else if(!ot_traffic_aggregator_add(&e->traffic,scope,ts,n,0))return 0;
 e->total_packets++;e->total_bytes+=n;e->health.input_packets++;
 if(d.state==OT_PARSE_PARTIAL||d.state==OT_PARSE_UNPARSEABLE)e->health.parser_rejects++;
 return 1;
}
