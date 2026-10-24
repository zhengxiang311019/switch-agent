#ifndef OT_ANALYZER_TIMEBASE_H
#define OT_ANALYZER_TIMEBASE_H

#include <stdint.h>

typedef enum ot_timebase_state { OT_TIME_STABLE, OT_TIME_CHANGED, OT_TIME_UNKNOWN } ot_timebase_state_t;
typedef struct ot_timebase { uint64_t last_ns; uint64_t counter_epoch; ot_timebase_state_t state; } ot_timebase_t;
void ot_timebase_init(ot_timebase_t *timebase);
ot_timebase_state_t ot_timebase_observe(ot_timebase_t *timebase, uint64_t timestamp_ns);
void ot_timebase_reset_epoch(ot_timebase_t *timebase);

#endif
