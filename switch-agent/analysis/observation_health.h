#ifndef OT_ANALYZER_OBSERVATION_HEALTH_H
#define OT_ANALYZER_OBSERVATION_HEALTH_H
#include <stdint.h>
typedef enum ot_completeness { OT_COMPLETE, OT_DEGRADED, OT_COMPLETENESS_UNKNOWN } ot_completeness_t;
typedef struct ot_observation_health { uint64_t input_packets,input_drops,parser_rejects,truncated_packets,resource_drops; uint64_t counter_epoch; ot_completeness_t completeness; } ot_observation_health_t;
void ot_observation_health_init(ot_observation_health_t *health);
void ot_observation_health_update(ot_observation_health_t *health,uint64_t input_packets,uint64_t input_drops,uint64_t parser_rejects,uint64_t truncated,uint64_t resource_drops,uint64_t epoch);
#endif
