#include "health.h"

#include <stddef.h>

void ot_capture_health_init(ot_capture_health_t *health)
{
    if (health != NULL) {
        *health = (ot_capture_health_t){0};
    }
}

void ot_capture_health_record_invalid(ot_capture_health_t *health)
{
    if (health != NULL) {
        health->invalid_records++;
    }
}

void ot_capture_health_record_truncated(ot_capture_health_t *health)
{
    if (health != NULL) {
        health->truncated_packets++;
    }
}

void ot_capture_health_record_resource_drop(ot_capture_health_t *health)
{
    if (health != NULL) {
        health->resource_pressure_drops++;
    }
}

void ot_capture_health_record_source_drop(ot_capture_health_t *health)
{
    if (health != NULL) {
        health->source_drops++;
    }
}
