#include "quality.h"
#include <string.h>
void ot_quality_init(ot_quality_tracker_t *t,uint64_t timeout){if(t){memset(t,0,sizeof(*t));t->timeout_ns=timeout;t->stats.rtt_min_ns=UINT64_MAX;}}
int ot_quality_transaction(ot_quality_tracker_t *t,uint64_t key,uint64_t ts,int response)
{
 if(!t||!key||!ts)return 0;
 for(size_t i=0;i<OT_QUALITY_PENDING_CAPACITY;i++){ot_quality_pending_t *p=&t->pending[i];if(p->used&&p->key==key){if(response&&ts>=p->time_ns){uint64_t rtt=ts-p->time_ns;if(t->stats.rtt_count){uint64_t prev=t->stats.last_rtt_ns;t->stats.jitter_sum_ns+=rtt>prev?rtt-prev:prev-rtt;t->stats.jitter_count++;}t->stats.last_rtt_ns=rtt;t->stats.rtt_count++;t->stats.rtt_sum_ns+=rtt;if(rtt<t->stats.rtt_min_ns)t->stats.rtt_min_ns=rtt;if(rtt>t->stats.rtt_max_ns)t->stats.rtt_max_ns=rtt;p->used=0;return 1;}return 0;}}
 if (response) return 0;
 for (size_t i=0;i<OT_QUALITY_PENDING_CAPACITY;i++) {
  if (!t->pending[i].used) { t->pending[i].used=1; t->pending[i].key=key; t->pending[i].time_ns=ts; return 1; }
 }
 return 0;
}
void ot_quality_expire(ot_quality_tracker_t *t,uint64_t now){if(!t)return;for(size_t i=0;i<OT_QUALITY_PENDING_CAPACITY;i++){ot_quality_pending_t *p=&t->pending[i];if(p->used&&now>=p->time_ns&&now-p->time_ns>=t->timeout_ns){p->used=0;t->stats.timeout_count++;}}}
void ot_quality_checksum(ot_quality_tracker_t *t,int visible,int valid){if(t&&visible&&!valid)t->stats.checksum_failures++;}
void ot_quality_sequence_gap(ot_quality_tracker_t *t,int visible,int complete){if(t&&visible&&complete)t->stats.sequence_gap_count++;}
