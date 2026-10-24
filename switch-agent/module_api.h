#ifndef OT_ANALYZER_MODULE_API_H
#define OT_ANALYZER_MODULE_API_H

#include "platform/hal_capture.h"
#include "analysis/tcp_analysis.h"
#include "presentation/monitoring_status.h"
#include "store/retention.h"

/* Module boundary: host provides HAL and authorization decisions; the module
 * owns protocol analysis and returns typed snapshots. No vendor/UI dependency.
 */
typedef struct ot_module ot_module_t;
typedef struct ot_tcp_diagnostics_summary {
 size_t active_flows;
 uint64_t flow_capacity_drops;
 uint64_t rto_ns;
} ot_tcp_diagnostics_summary_t;
ot_module_t *ot_module_create(uint32_t scope_id,uint64_t bucket_width_ns,uint32_t max_retention_days);
void ot_module_destroy(ot_module_t *module);
ot_hal_status_t ot_module_poll(ot_module_t *module,const ot_hal_capture_t *hal,uint32_t timeout_ms);
int ot_module_ingest(ot_module_t *module,const uint8_t *frame,size_t length,uint64_t timestamp_ns);
int ot_module_set_retention(ot_module_t *module,uint32_t days,int caller_is_admin);
void ot_module_set_capture_coverage(ot_module_t *module,ot_capture_coverage_t coverage);
void ot_module_set_tcp_rto(ot_module_t *module,uint64_t rto_ns);
void ot_module_tick(ot_module_t *module,uint64_t monotonic_now_ns);
size_t ot_module_get_tcp_diagnostics(const ot_module_t *module,ot_tcp_flow_snapshot_t *out,size_t capacity);
void ot_module_get_tcp_summary(const ot_module_t *module,ot_tcp_diagnostics_summary_t *summary);
void ot_module_get_overview(const ot_module_t *module,ot_overview_snapshot_t *snapshot);

#endif
