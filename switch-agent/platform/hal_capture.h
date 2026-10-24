#ifndef OT_ANALYZER_HAL_CAPTURE_H
#define OT_ANALYZER_HAL_CAPTURE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Platform-neutral packet receive contract used by the OT analyzer.
 * Implementations adapt vendor SDK/driver details to these records.
 */
typedef enum ot_hal_status {
    OT_HAL_OK = 0,
    OT_HAL_TIMEOUT = 1,
    OT_HAL_UNAVAILABLE = 2,
    OT_HAL_ERROR = 3
} ot_hal_status_t;

enum ot_hal_metadata_flags {
    OT_HAL_META_INGRESS_PORT = 1u << 0,
    OT_HAL_META_DIRECTION = 1u << 1,
    OT_HAL_META_TIMESTAMP_NS = 1u << 2,
    OT_HAL_META_CHECKSUM_STATUS = 1u << 3,
    OT_HAL_META_SOURCE_COUNTERS = 1u << 4
};

typedef struct ot_hal_packet {
    const uint8_t *frame;
    size_t captured_length;
    size_t wire_length;
    uint32_t metadata_flags;
    uint32_t ingress_port;
    uint8_t ingress_direction;
    uint64_t timestamp_ns;
    uint8_t checksum_valid;
} ot_hal_packet_t;

typedef struct ot_hal_source_counters {
    uint64_t received_packets;
    uint64_t dropped_packets;
    uint32_t available_flags;
} ot_hal_source_counters_t;

typedef struct ot_hal_capture {
    void *context;
    ot_hal_status_t (*receive)(void *context, uint32_t timeout_ms,
                               ot_hal_packet_t *packet);
    ot_hal_status_t (*get_counters)(void *context,
                                    ot_hal_source_counters_t *counters);
} ot_hal_capture_t;

/* Validates one HAL record. Returned frame memory remains owned by the HAL
 * and is valid only until its next receive() call.
 */
ot_hal_status_t ot_hal_receive_one(const ot_hal_capture_t *capture,
                                   uint32_t timeout_ms,
                                   ot_hal_packet_t *packet);

/* The analyzer borrows frame storage only until receive() is called again.
 * A missing metadata flag means unavailable; consumers must not infer a value.
 */

#ifdef __cplusplus
}
#endif

#endif /* OT_ANALYZER_HAL_CAPTURE_H */
