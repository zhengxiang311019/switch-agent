#ifndef OT_ANALYZER_RETENTION_H
#define OT_ANALYZER_RETENTION_H
#include <stdint.h>
#define OT_RETENTION_DEFAULT_DAYS 7U
typedef struct ot_retention_settings { uint32_t days; uint32_t device_max_days; } ot_retention_settings_t;
void ot_retention_init(ot_retention_settings_t *settings,uint32_t device_max_days);
int ot_retention_set_days(ot_retention_settings_t *settings,uint32_t days,int is_admin);
int ot_retention_expired(uint64_t record_time_ns,uint64_t now_ns,const ot_retention_settings_t *settings);
#endif
