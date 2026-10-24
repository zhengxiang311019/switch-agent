#ifndef OT_ANALYZER_QUALITY_H
#define OT_ANALYZER_QUALITY_H
#include <stddef.h>
#include <stdint.h>
#define OT_QUALITY_PENDING_CAPACITY 512U
typedef struct ot_quality_pending { uint64_t key,time_ns; uint8_t used; } ot_quality_pending_t;
typedef struct ot_quality_stats { uint64_t rtt_count,rtt_sum_ns,rtt_min_ns,rtt_max_ns,timeout_count,sequence_gap_count,checksum_failures; uint64_t last_rtt_ns,jitter_sum_ns,jitter_count; } ot_quality_stats_t;
typedef struct ot_quality_tracker { ot_quality_pending_t pending[OT_QUALITY_PENDING_CAPACITY]; ot_quality_stats_t stats; uint64_t timeout_ns; } ot_quality_tracker_t;
void ot_quality_init(ot_quality_tracker_t *tracker,uint64_t timeout_ns);
int ot_quality_transaction(ot_quality_tracker_t *tracker,uint64_t key,uint64_t timestamp_ns,int response);
void ot_quality_expire(ot_quality_tracker_t *tracker,uint64_t now_ns);
void ot_quality_checksum(ot_quality_tracker_t *tracker,int visible,int valid);
void ot_quality_sequence_gap(ot_quality_tracker_t *tracker,int sequence_visible,int capture_complete);
#endif
