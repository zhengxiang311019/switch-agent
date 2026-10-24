#ifndef OT_ANALYZER_PACKET_QUEUE_H
#define OT_ANALYZER_PACKET_QUEUE_H

#include "../platform/hal_capture.h"
#include "health.h"

#include <stddef.h>
#include <stdint.h>

typedef struct ot_packet_slot {
    uint8_t *storage;
    size_t capacity;
    ot_hal_packet_t packet;
} ot_packet_slot_t;

typedef struct ot_packet_queue {
    ot_packet_slot_t *slots;
    size_t slot_count;
    size_t head;
    size_t count;
} ot_packet_queue_t;

int ot_packet_queue_init(ot_packet_queue_t *queue, ot_packet_slot_t *slots,
                         size_t slot_count);
int ot_packet_queue_push(ot_packet_queue_t *queue,
                         const ot_hal_packet_t *packet,
                         ot_capture_health_t *health);
int ot_packet_queue_pop(ot_packet_queue_t *queue, ot_hal_packet_t *packet);

#endif
