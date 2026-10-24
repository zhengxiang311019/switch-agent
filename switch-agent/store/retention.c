#include "retention.h"
#include <stddef.h>
void ot_retention_init(ot_retention_settings_t *s,uint32_t max){if(s){s->device_max_days=max<OT_RETENTION_DEFAULT_DAYS?OT_RETENTION_DEFAULT_DAYS:max;s->days=OT_RETENTION_DEFAULT_DAYS;}}
int ot_retention_set_days(ot_retention_settings_t *s,uint32_t days,int admin){if(!s||!admin||days<OT_RETENTION_DEFAULT_DAYS||days>s->device_max_days)return 0;s->days=days;return 1;}
int ot_retention_expired(uint64_t then,uint64_t now,const ot_retention_settings_t *s){if(!s||now<then)return 0;uint64_t ns_per_day=86400000000000ULL;return now-then>(uint64_t)s->days*ns_per_day;}
