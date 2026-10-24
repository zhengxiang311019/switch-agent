#ifndef OT_ANALYZER_FLOW_CATALOG_H
#define OT_ANALYZER_FLOW_CATALOG_H

#include "../protocols/protocols.h"
#include <stddef.h>
#include <stdint.h>

#define OT_FLOW_CATALOG_CAPACITY 256U
typedef struct ot_flow_key { uint8_t src_mac[6], dst_mac[6],ip_version,src_ip[16],dst_ip[16],vlan_count; uint16_t vlan_id[4],protocol_id; uint8_t ip_protocol; uint16_t src_port, dst_port; } ot_flow_key_t;
typedef struct ot_flow_entry { ot_flow_key_t key; uint64_t first_seen_ns, last_seen_ns, packet_count, byte_count; ot_protocol_t protocol; ot_business_class_t business_class; ot_parse_state_t parse_state; uint8_t used; } ot_flow_entry_t;
typedef struct ot_flow_catalog { ot_flow_entry_t entries[OT_FLOW_CATALOG_CAPACITY]; size_t count; } ot_flow_catalog_t;
void ot_flow_catalog_init(ot_flow_catalog_t *catalog);
ot_flow_entry_t *ot_flow_catalog_observe(ot_flow_catalog_t *catalog, const ot_flow_key_t *key, const ot_decode_result_t *decoded, uint64_t timestamp_ns, size_t bytes);

#endif
