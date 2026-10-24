#include "hal_capture.h"

ot_hal_status_t ot_hal_receive_one(const ot_hal_capture_t *capture,
                                   uint32_t timeout_ms,
                                   ot_hal_packet_t *packet)
{
    if (capture == NULL || capture->receive == NULL || packet == NULL) {
        return OT_HAL_ERROR;
    }

    packet->frame = NULL;
    packet->captured_length = 0U;
    packet->wire_length = 0U;
    packet->metadata_flags = 0U;

    const ot_hal_status_t status =
        capture->receive(capture->context, timeout_ms, packet);
    if (status != OT_HAL_OK) {
        return status;
    }
    if (packet->frame == NULL || packet->captured_length == 0U ||
        packet->wire_length < packet->captured_length) {
        packet->frame = NULL;
        packet->captured_length = 0U;
        return OT_HAL_ERROR;
    }
    return OT_HAL_OK;
}
