#include "observation_health.h"
#include <stddef.h>
#include <string.h>
void ot_observation_health_init(ot_observation_health_t *h){if(h){memset(h,0,sizeof(*h));h->completeness=OT_COMPLETENESS_UNKNOWN;}}
void ot_observation_health_update(ot_observation_health_t *h,uint64_t packets,uint64_t drops,uint64_t rejects,uint64_t truncated,uint64_t resource,uint64_t epoch)
{
 if (!h) return;
 h->input_packets=packets; h->input_drops=drops; h->parser_rejects=rejects;
 h->truncated_packets=truncated; h->resource_drops=resource;
 if(h->counter_epoch!=0U&&epoch!=h->counter_epoch){h->completeness=OT_DEGRADED;}else if(drops||rejects||truncated||resource){h->completeness=OT_DEGRADED;}else{h->completeness=OT_COMPLETE;} h->counter_epoch=epoch;
}
