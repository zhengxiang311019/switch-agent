#include "monitoring_status.h"
#include <string.h>
void ot_monitoring_status_snapshot(const ot_analyzer_engine_t *e,int enabled,ot_overview_snapshot_t *o)
{
 if (!o) return;
 memset(o,0,sizeof(*o));
 if(!enabled){o->state=OT_UI_DISABLED;return;}
 if(!e){o->state=OT_UI_UNAVAILABLE;o->completeness=OT_COMPLETENESS_UNKNOWN;return;}
 o->packets=e->total_packets;o->bytes=e->total_bytes;o->flow_count=e->flows.count;o->completeness=e->health.completeness;
 for(size_t i=0;i<OT_AGGREGATE_CAPACITY;i++){const ot_traffic_bucket_t *b=&e->traffic.buckets[i];if(b->used){o->known_packets+=b->known_packets;o->unknown_packets+=b->unknown_packets;}}
 o->state=e->total_packets==0?OT_UI_NO_TRAFFIC:(e->health.completeness==OT_DEGRADED?OT_UI_DEGRADED:OT_UI_READY);
}
