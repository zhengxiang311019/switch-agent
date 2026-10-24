#include "protocols.h"
#include <string.h>

static uint16_t be16(const uint8_t *p) { return (uint16_t)(((uint16_t)p[0] << 8) | p[1]); }
static ot_decode_result_t out(ot_parse_state_t s, ot_business_class_t c, size_t n) {
    ot_decode_result_t r; memset(&r, 0, sizeof(r)); r.protocol=OT_PROTO_MODBUS_TCP;
    r.state=s; r.business_class=c; r.payload_offset=7U; r.payload_length=n>7U?n-7U:0U; return r;
}

ot_decode_result_t ot_decode_modbus_tcp(const uint8_t *p, size_t n) {
    if (!p || n < 7U) return out(OT_PARSE_PARTIAL, OT_CLASS_UNKNOWN, n);
    uint16_t pid=be16(p+2), mb_len=be16(p+4);
    if (pid != 0U) return out(OT_PARSE_UNPARSEABLE, OT_CLASS_UNKNOWN, n);
    if (mb_len < 2U || mb_len > 254U) return out(OT_PARSE_UNPARSEABLE, OT_CLASS_UNKNOWN, n);
    if ((size_t)mb_len > n-6U) return out(OT_PARSE_PARTIAL, OT_CLASS_UNKNOWN, n);
    ot_decode_result_t r=out(OT_PARSE_OK, OT_CLASS_MODBUS_DIAGNOSTIC, n);
    r.transaction_id=be16(p); r.protocol_id=pid; r.declared_length=mb_len; r.unit_or_status=p[6];
    uint8_t fc=p[7]; r.function_or_message_type=fc; r.service_or_frame_id=fc;
    if ((fc & 0x80U)!=0U) { if (mb_len < 3U) r.state=OT_PARSE_PARTIAL; else r.unit_or_status=p[8]; }
    else if (fc==1U||fc==2U||fc==3U||fc==4U) r.business_class=OT_CLASS_MODBUS_READ;
    else if (fc==5U||fc==6U||fc==15U||fc==16U||fc==22U||fc==23U) r.business_class=OT_CLASS_MODBUS_WRITE;
    if (mb_len < 2U) r.state=OT_PARSE_PARTIAL;
    return r;
}
