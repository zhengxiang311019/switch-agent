#include "packet_queue.h"

#include <string.h>

int ot_packet_queue_init(ot_packet_queue_t *queue, ot_packet_slot_t *slots,
                         size_t slot_count)
{
    if (queue == NULL || slots == NULL || slot_count == 0U) {
        return 0;
    }
    for (size_t i = 0; i < slot_count; ++i) {
        if (slots[i].storage == NULL || slots[i].capacity == 0U) {
            return 0;
        }
    }
    queue->slots = slots;
    queue->slot_count = slot_count;
    queue->head = 0U;
    queue->count = 0U;
    return 1;
}

int ot_packet_queue_push(ot_packet_queue_t *queue,
                         const ot_hal_packet_t *packet,
                         ot_capture_health_t *health)
{
    if (queue == NULL || packet == NULL || queue->slots == NULL ||
        packet->frame == NULL || packet->captured_length == 0U) {
        ot_capture_health_record_invalid(health);
        return 0;
    }
    if (queue->count == queue->slot_count) {
        ot_capture_health_record_resource_drop(health);
        return 0;
    }
    const size_t tail = (queue->head + queue->count) % queue->slot_count;
    ot_packet_slot_t *slot = &queue->slots[tail];
    if (packet->captured_length > slot->capacity) {
        ot_capture_health_record_truncated(health);
        return 0;
    }
    memcpy(slot->storage, packet->frame, packet->captured_length);
    slot->packet = *packet;
    slot->packet.frame = slot->storage;
    queue->count++;
    if (health != NULL) {
        health->received_packets++;
    }
    return 1;
}

int ot_packet_queue_pop(ot_packet_queue_t *queue, ot_hal_packet_t *packet)
{
    if (queue == NULL || packet == NULL || queue->count == 0U) {
        return 0;
    }
    *packet = queue->slots[queue->head].packet;
    queue->head = (queue->head + 1U) % queue->slot_count;
    queue->count--;
    return 1;
}
