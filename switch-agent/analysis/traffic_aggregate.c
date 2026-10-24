#include "traffic_aggregate.h"
#include <string.h>
void ot_traffic_aggregator_init(ot_traffic_aggregator_t *a,uint64_t width) { if(a){memset(a,0,sizeof(*a));a->bucket_width_ns=width;} }
int ot_traffic_aggregator_add(ot_traffic_aggregator_t *a,uint32_t scope,uint64_t ts,size_t bytes,int recognized)
{
 if (!a || !a->bucket_width_ns) return 0;
 uint64_t start = (ts / a->bucket_width_ns) * a->bucket_width_ns;
 for(size_t i=0;i<OT_AGGREGATE_CAPACITY;i++){ot_traffic_bucket_t *b=&a->buckets[i];if(b->used&&b->scope_id==scope&&b->bucket_start_ns==start){b->packets++;b->bytes+=bytes;if(recognized)b->known_packets++;else b->unknown_packets++;return 1;}}
 for(size_t i=0;i<OT_AGGREGATE_CAPACITY;i++){ot_traffic_bucket_t *b=&a->buckets[i];if(!b->used){b->used=1;b->scope_id=scope;b->bucket_start_ns=start;b->packets=1;b->bytes=bytes;b->known_packets=recognized?1U:0U;b->unknown_packets=recognized?0U:1U;return 1;}} return 0;
}
