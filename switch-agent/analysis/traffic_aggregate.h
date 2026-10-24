#ifndef OT_ANALYZER_TRAFFIC_AGGREGATE_H
#define OT_ANALYZER_TRAFFIC_AGGREGATE_H
#include <stddef.h>
#include <stdint.h>
#define OT_AGGREGATE_CAPACITY 512U
typedef struct ot_traffic_bucket { uint64_t bucket_start_ns, packets, bytes, known_packets, unknown_packets; uint32_t scope_id; uint8_t used; } ot_traffic_bucket_t;
typedef struct ot_traffic_aggregator { ot_traffic_bucket_t buckets[OT_AGGREGATE_CAPACITY]; uint64_t bucket_width_ns; } ot_traffic_aggregator_t;
void ot_traffic_aggregator_init(ot_traffic_aggregator_t *agg,uint64_t bucket_width_ns);
int ot_traffic_aggregator_add(ot_traffic_aggregator_t *agg,uint32_t scope_id,uint64_t timestamp_ns,size_t bytes,int recognized);
#endif
