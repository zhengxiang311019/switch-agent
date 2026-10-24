#ifndef OT_ANALYZER_PROTOCOLS_H
#define OT_ANALYZER_PROTOCOLS_H

#include <stddef.h>
#include <stdint.h>

typedef enum ot_protocol { OT_PROTO_UNKNOWN, OT_PROTO_ENIP, OT_PROTO_PROFINET,
    OT_PROTO_MODBUS_TCP, OT_PROTO_OPC_UA, OT_PROTO_MQTT } ot_protocol_t;
typedef enum ot_parse_state { OT_PARSE_OK, OT_PARSE_PARTIAL, OT_PARSE_UNPARSEABLE } ot_parse_state_t;
typedef enum ot_business_class {
    OT_CLASS_UNKNOWN, OT_CLASS_ENIP_EXPLICIT, OT_CLASS_ENIP_IMPLICIT,
    OT_CLASS_PN_DCP, OT_CLASS_PN_ALARM, OT_CLASS_PN_REALTIME,
    OT_CLASS_MODBUS_READ, OT_CLASS_MODBUS_WRITE, OT_CLASS_MODBUS_DIAGNOSTIC,
    OT_CLASS_OPCUA_HEL, OT_CLASS_OPCUA_SECURE, OT_CLASS_MQTT_CONTROL,
    OT_CLASS_MQTT_PUBLISH, OT_CLASS_ENCRYPTED
} ot_business_class_t;

typedef struct ot_decode_result {
    ot_protocol_t protocol;
    ot_parse_state_t state;
    ot_business_class_t business_class;
    uint16_t protocol_id;
    uint16_t service_or_frame_id;
    uint8_t cip_service, cip_general_status, pn_dcp_service;
    uint32_t pn_dcp_xid;
    uint16_t pn_dcp_data_length;
    uint8_t pn_dcp_blocks;
    uint8_t function_or_message_type;
    uint8_t quality_of_service;
    uint8_t transaction_flags;
    uint16_t transaction_id;
    uint16_t unit_or_status;
    uint32_t declared_length;
    uint32_t session_id;
    uint32_t io_sequence;
    size_t payload_offset;
    size_t payload_length;
} ot_decode_result_t;

#define OT_PACKET_MAX_VLAN_TAGS 4U
typedef struct ot_packet_view {
    uint8_t ip_version, transport_protocol, vlan_count, fragmented;
    uint16_t vlan_id[OT_PACKET_MAX_VLAN_TAGS];
    uint8_t src_mac[6], dst_mac[6], src_ip[16], dst_ip[16];
    uint16_t src_port, dst_port, tcp_window, tcp_mss;
    uint8_t tcp_window_scale, tcp_sack_permitted, tcp_sack_blocks, tcp_options_valid;
    uint32_t tcp_timestamp_value,tcp_timestamp_echo;
    uint32_t tcp_sequence, tcp_acknowledgment;
    uint8_t tcp_flags;
    size_t l3_offset, l4_offset, payload_offset, payload_length;
} ot_packet_view_t;

/* Decodes Ethernet II with up to two 802.1Q/802.1ad tags. */
ot_decode_result_t ot_protocol_decode(const uint8_t *frame, size_t length);
int ot_packet_view_decode(const uint8_t *frame, size_t length, ot_packet_view_t *view);
ot_decode_result_t ot_decode_enip(const uint8_t *data, size_t length,
                                  uint16_t transport_port);
ot_decode_result_t ot_decode_profinet(const uint8_t *data, size_t length,
                                      size_t frame_offset);
ot_decode_result_t ot_decode_modbus_tcp(const uint8_t *data, size_t length);
ot_decode_result_t ot_decode_opcua(const uint8_t *data, size_t length);
ot_decode_result_t ot_decode_mqtt(const uint8_t *data, size_t length, int tls_port);

#endif
