#ifndef OT_ANALYZER_CAPTURE_HEALTH_H
#define OT_ANALYZER_CAPTURE_HEALTH_H

#include <stdint.h>

typedef struct ot_capture_health {
    uint64_t received_packets;
    uint64_t source_drops;
    uint64_t invalid_records;
    uint64_t truncated_packets;
    uint64_t resource_pressure_drops;
} ot_capture_health_t;

void ot_capture_health_init(ot_capture_health_t *health);
void ot_capture_health_record_invalid(ot_capture_health_t *health);
void ot_capture_health_record_truncated(ot_capture_health_t *health);
void ot_capture_health_record_resource_drop(ot_capture_health_t *health);
void ot_capture_health_record_source_drop(ot_capture_health_t *health);

#endif
