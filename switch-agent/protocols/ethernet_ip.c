#include "protocols.h"

#include <string.h>

static uint16_t le16(const uint8_t *p) { return (uint16_t)(p[0] | ((uint16_t)p[1] << 8)); }
static uint32_t le32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }
static uint16_t be16(const uint8_t *p) { return (uint16_t)(((uint16_t)p[0] << 8) | p[1]); }

static ot_decode_result_t result(ot_protocol_t proto, ot_parse_state_t state,
                                 ot_business_class_t cls, size_t off, size_t len)
{
    ot_decode_result_t r;
    memset(&r, 0, sizeof(r));
    r.protocol = proto; r.state = state; r.business_class = cls;
    r.payload_offset = off; r.payload_length = len;
    return r;
}

ot_decode_result_t ot_decode_enip(const uint8_t *data, size_t length,
                                 uint16_t transport_port)
{
    if (transport_port != 44818U && transport_port != 2222U)
        return result(OT_PROTO_UNKNOWN, OT_PARSE_UNPARSEABLE, OT_CLASS_UNKNOWN, 0, 0);
    /* UDP/2222 carries real-time I/O packets, not the 24-byte TCP encapsulation header. */
    if (transport_port == 2222U) {
        if (data == NULL || length < 4U)
            return result(OT_PROTO_ENIP, OT_PARSE_PARTIAL, OT_CLASS_ENIP_IMPLICIT, 4U, 0);
        ot_decode_result_t io = result(OT_PROTO_ENIP, OT_PARSE_OK, OT_CLASS_ENIP_IMPLICIT, 4U, length - 4U);
        io.protocol_id = 0x8002U; /* Unconnected data item framing. */
        io.io_sequence = le32(data);
        return io;
    }
    if (data == NULL)
        return result(OT_PROTO_ENIP, OT_PARSE_UNPARSEABLE, OT_CLASS_UNKNOWN, 0, 0);
    if (length < 24U)
        return result(OT_PROTO_ENIP, OT_PARSE_PARTIAL, OT_CLASS_UNKNOWN, 0, length);
    const uint16_t command = le16(data);
    const uint16_t encapsulated_length = le16(data + 2);
    if ((size_t)encapsulated_length > length - 24U)
        return result(OT_PROTO_ENIP, OT_PARSE_PARTIAL, OT_CLASS_UNKNOWN, 24U, length - 24U);
    ot_decode_result_t r = result(OT_PROTO_ENIP, OT_PARSE_OK, OT_CLASS_ENIP_EXPLICIT,
                                  24U, encapsulated_length);
    r.protocol_id = command;
    r.session_id = le32(data + 4);
    if (command == 0x006fU || command == 0x0070U) {
        /* SendRRData/SendUnitData: interface handle, timeout, item count. */
        if (encapsulated_length < 8U)
            return result(OT_PROTO_ENIP, OT_PARSE_PARTIAL, OT_CLASS_UNKNOWN, 24U, encapsulated_length);
        const uint16_t count = le16(data + 30);
        size_t pos = 32U;
        ot_business_class_t found = command == 0x0070U ? OT_CLASS_ENIP_IMPLICIT : OT_CLASS_ENIP_EXPLICIT;
        for (uint16_t i = 0; i < count; ++i) {
            if (pos + 4U > 24U + encapsulated_length)
                return result(OT_PROTO_ENIP, OT_PARSE_PARTIAL, OT_CLASS_UNKNOWN, pos, 0);
            const uint16_t type = le16(data + pos), item_len = le16(data + pos + 2U);
            pos += 4U;
            if ((size_t)item_len > 24U + encapsulated_length - pos)
                return result(OT_PROTO_ENIP, OT_PARSE_PARTIAL, OT_CLASS_UNKNOWN, pos, 0);
            if (type == 0x00b2U && item_len >= 2U) {
                r.service_or_frame_id = data[pos + 1U] & 0x7fU;
                if (item_len >= 5U) { r.cip_service=data[pos+2U]; r.cip_general_status=data[pos+4U]; }
            }
            if (type == 0x00b1U && item_len >= 2U) { found = OT_CLASS_ENIP_IMPLICIT; r.service_or_frame_id = data[pos + 1U] & 0x7fU; }
            pos += item_len;
        }
        r.business_class = found; r.payload_offset = pos;
        r.payload_length = 24U + encapsulated_length - pos;
    }
    return r;
}

static uint16_t frame_ethertype(const uint8_t *frame,size_t length,size_t *payload)
{
    if(length<14U)return 0;
    size_t off=14U; uint16_t type=be16(frame+12U);
    while((type==0x8100U||type==0x88a8U||type==0x9100U)&&length>=off+4U){type=be16(frame+off+2U);off+=4U;}
    if(payload)*payload=off;
    return type;
}

static ot_decode_result_t ot_decode_ethernet(const uint8_t *frame, size_t length)
{
    ot_packet_view_t v;
    if(!ot_packet_view_decode(frame,length,&v))return result(OT_PROTO_UNKNOWN,OT_PARSE_PARTIAL,OT_CLASS_UNKNOWN,0,0);
    size_t l2=0;uint16_t et=frame_ethertype(frame,length,&l2);
    if(et==0x8892U)return ot_decode_profinet(frame+l2,length-l2,l2);
    if(v.ip_version==0U)return result(OT_PROTO_UNKNOWN,OT_PARSE_UNPARSEABLE,OT_CLASS_UNKNOWN,v.l3_offset,length>v.l3_offset?length-v.l3_offset:0U);
    size_t p=v.payload_offset,n=v.payload_length;uint16_t sport=v.src_port,dport=v.dst_port;
    if(v.transport_protocol==17U && (sport==2222U||dport==2222U))return ot_decode_enip(frame+p,n,2222U);
    if(v.transport_protocol!=6U)return result(OT_PROTO_UNKNOWN,OT_PARSE_UNPARSEABLE,OT_CLASS_UNKNOWN,p,n);
    uint16_t port=sport==44818U||dport==44818U?44818U:(sport==502U||dport==502U?502U:(sport==4840U||dport==4840U?4840U:(sport==8883U||dport==8883U?8883U:(sport==1883U||dport==1883U?1883U:0U))));
    if(port==44818U)return ot_decode_enip(frame+p,n,port);
    if(port==502U)return ot_decode_modbus_tcp(frame+p,n);
    if(port==4840U)return ot_decode_opcua(frame+p,n);
    if(port==1883U||port==8883U)return ot_decode_mqtt(frame+p,n,port==8883U);
    return result(OT_PROTO_UNKNOWN,OT_PARSE_UNPARSEABLE,OT_CLASS_UNKNOWN,p,n);
}

ot_decode_result_t ot_protocol_decode(const uint8_t *frame, size_t length)
{
    return ot_decode_ethernet(frame, length);
}
