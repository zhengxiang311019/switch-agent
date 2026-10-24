#include "flow_catalog.h"
#include <string.h>

void ot_flow_catalog_init(ot_flow_catalog_t *c) { if (c) memset(c,0,sizeof(*c)); }
static int key_equal(const ot_flow_key_t *a,const ot_flow_key_t *b)
{ return memcmp(a->src_mac,b->src_mac,6)==0 && memcmp(a->dst_mac,b->dst_mac,6)==0 && a->ip_version==b->ip_version && memcmp(a->src_ip,b->src_ip,16)==0 && memcmp(a->dst_ip,b->dst_ip,16)==0 && a->vlan_count==b->vlan_count && memcmp(a->vlan_id,b->vlan_id,sizeof(a->vlan_id))==0 && a->protocol_id==b->protocol_id && a->ip_protocol==b->ip_protocol && a->src_port==b->src_port && a->dst_port==b->dst_port; }
ot_flow_entry_t *ot_flow_catalog_observe(ot_flow_catalog_t *c,const ot_flow_key_t *k,const ot_decode_result_t *d,uint64_t ts,size_t bytes)
{
    if (!c||!k||!d) return NULL;
    for(size_t i=0;i<c->count;i++) if(key_equal(&c->entries[i].key,k)) { ot_flow_entry_t *e=&c->entries[i]; e->last_seen_ns=ts; e->packet_count++; e->byte_count+=bytes; e->protocol=d->protocol; e->business_class=d->business_class; e->parse_state=d->state; return e; }
    if(c->count>=OT_FLOW_CATALOG_CAPACITY) return NULL;
    ot_flow_entry_t *e=&c->entries[c->count++]; memset(e,0,sizeof(*e)); e->used=1; e->key=*k; e->first_seen_ns=e->last_seen_ns=ts; e->packet_count=1; e->byte_count=bytes; e->protocol=d->protocol; e->business_class=d->business_class; e->parse_state=d->state; return e;
}
