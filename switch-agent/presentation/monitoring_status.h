#ifndef OT_ANALYZER_MONITORING_STATUS_H
#define OT_ANALYZER_MONITORING_STATUS_H
#include "../analysis/engine.h"
typedef enum ot_ui_capture_state { OT_UI_DISABLED, OT_UI_NO_TRAFFIC, OT_UI_READY, OT_UI_DEGRADED, OT_UI_UNAVAILABLE } ot_ui_capture_state_t;
typedef struct ot_overview_snapshot { uint64_t packets,bytes,flow_count; uint64_t known_packets,unknown_packets; ot_completeness_t completeness; ot_ui_capture_state_t state; } ot_overview_snapshot_t;
void ot_monitoring_status_snapshot(const ot_analyzer_engine_t *engine,int enabled,ot_overview_snapshot_t *out);
#endif
