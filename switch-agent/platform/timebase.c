#include "timebase.h"

#include <stddef.h>

void ot_timebase_init(ot_timebase_t *tb) { if (tb) { tb->last_ns=0; tb->counter_epoch=0; tb->state=OT_TIME_UNKNOWN; } }
ot_timebase_state_t ot_timebase_observe(ot_timebase_t *tb, uint64_t now)
{
    if (!tb || now == 0U) return OT_TIME_UNKNOWN;
    if (tb->last_ns != 0U && now < tb->last_ns) { tb->counter_epoch++; tb->state=OT_TIME_CHANGED; }
    else if (tb->state == OT_TIME_UNKNOWN) tb->state=OT_TIME_STABLE;
    else if (tb->state == OT_TIME_CHANGED) tb->state=OT_TIME_STABLE;
    tb->last_ns=now;
    return tb->state;
}
void ot_timebase_reset_epoch(ot_timebase_t *tb) { if (tb) { tb->counter_epoch++; tb->state=OT_TIME_CHANGED; tb->last_ns=0U; } }
