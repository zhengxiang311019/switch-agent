#ifndef OT_ANALYZER_ENGINE_H
#define OT_ANALYZER_ENGINE_H
#include "flow_catalog.h"
#include "traffic_aggregate.h"
#include "observation_health.h"
#include "tcp_analysis.h"
typedef struct ot_analyzer_engine { ot_flow_catalog_t flows; ot_traffic_aggregator_t traffic; ot_observation_health_t health; ot_tcp_tracker_t tcp; ot_capture_coverage_t capture_coverage; uint64_t total_packets,total_bytes; } ot_analyzer_engine_t;
void ot_analyzer_engine_init(ot_analyzer_engine_t *engine,uint64_t bucket_width_ns);
void ot_analyzer_engine_set_capture_coverage(ot_analyzer_engine_t *engine,ot_capture_coverage_t coverage);
int ot_analyzer_ingest(ot_analyzer_engine_t *engine,uint32_t scope_id,const uint8_t *frame,size_t length,uint64_t timestamp_ns);
#endif
