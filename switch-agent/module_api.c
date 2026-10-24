#include "module_api.h"
#include "analysis/engine.h"
#include <stdlib.h>
#include <string.h>

struct ot_module { ot_analyzer_engine_t engine; ot_retention_settings_t retention; uint32_t scope_id; int enabled; };

ot_module_t *ot_module_create(uint32_t scope,uint64_t width,uint32_t max_days)
{
 if(!width)return NULL;
 ot_module_t *m=(ot_module_t *)calloc(1U,sizeof(*m));if(!m)return NULL;
 m->scope_id=scope;m->enabled=1;ot_analyzer_engine_init(&m->engine,width);ot_retention_init(&m->retention,max_days);return m;
}
void ot_module_destroy(ot_module_t *m)
{
 if(!m)return;
 volatile unsigned char *p=(volatile unsigned char *)m;for(size_t i=0;i<sizeof(*m);i++)p[i]=0U;free(m);
}
int ot_module_ingest(ot_module_t *m,const uint8_t *frame,size_t n,uint64_t ts)
{return m&&m->enabled?ot_analyzer_ingest(&m->engine,m->scope_id,frame,n,ts):0;}
ot_hal_status_t ot_module_poll(ot_module_t *m,const ot_hal_capture_t *hal,uint32_t timeout)
{if(!m||!hal)return OT_HAL_ERROR;ot_hal_packet_t p;ot_hal_status_t s=ot_hal_receive_one(hal,timeout,&p);if(s==OT_HAL_OK&&!ot_module_ingest(m,p.frame,p.captured_length,(p.metadata_flags&OT_HAL_META_TIMESTAMP_NS)?p.timestamp_ns:0U))return OT_HAL_ERROR;return s;}
int ot_module_set_retention(ot_module_t *m,uint32_t days,int admin){return m&&ot_retention_set_days(&m->retention,days,admin);}
void ot_module_set_capture_coverage(ot_module_t *m,ot_capture_coverage_t coverage){if(m)ot_analyzer_engine_set_capture_coverage(&m->engine,coverage);}
void ot_module_set_tcp_rto(ot_module_t *m,uint64_t rto){if(m&&rto)m->engine.tcp.rto_ns=rto;}
void ot_module_tick(ot_module_t *m,uint64_t now){if(m&&m->enabled)ot_tcp_tracker_expire(&m->engine.tcp,now);}
size_t ot_module_get_tcp_diagnostics(const ot_module_t *m,ot_tcp_flow_snapshot_t *out,size_t cap)
{
 if(!m||!out||!cap)return 0;
 size_t n=m->engine.tcp.count<cap?m->engine.tcp.count:cap;
 for(size_t i=0;i<n;i++){const ot_tcp_flow_entry_t *e=&m->engine.tcp.flows[i];memset(&out[i],0,sizeof(out[i]));memcpy(out[i].src_ip,e->src_ip,16);memcpy(out[i].dst_ip,e->dst_ip,16);out[i].ip_version=e->ip_version;out[i].vlan_count=e->vlan_count;out[i].src_port=e->src_port;out[i].dst_port=e->dst_port;memcpy(out[i].vlan_id,e->vlan_id,sizeof(out[i].vlan_id));out[i].stats=e->stats;}
 return n;
}
void ot_module_get_tcp_summary(const ot_module_t *m,ot_tcp_diagnostics_summary_t *out)
{if(!out)return;memset(out,0,sizeof(*out));if(m){out->active_flows=m->engine.tcp.count;out->flow_capacity_drops=m->engine.tcp.flow_capacity_drops;out->rto_ns=m->engine.tcp.rto_ns;}}
void ot_module_get_overview(const ot_module_t *m,ot_overview_snapshot_t *out){if(m)ot_monitoring_status_snapshot(&m->engine,m->enabled,out);else ot_monitoring_status_snapshot(NULL,0,out);}
