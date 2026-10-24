#include "protocols.h"
#include <string.h>

static ot_decode_result_t out(ot_parse_state_t s, ot_business_class_t c, size_t n) {
    ot_decode_result_t r; memset(&r,0,sizeof(r)); r.protocol=OT_PROTO_MQTT; r.state=s;
    r.business_class=c; r.payload_offset=2U; r.payload_length=n>2U?n-2U:0U; return r;
}
ot_decode_result_t ot_decode_mqtt(const uint8_t *p, size_t n, int tls_port) {
    if (tls_port) { ot_decode_result_t r=out(n?OT_PARSE_OK:OT_PARSE_PARTIAL,OT_CLASS_ENCRYPTED,n); r.payload_offset=0; r.payload_length=n; return r; }
    if (!p||n<2U) return out(OT_PARSE_PARTIAL,OT_CLASS_UNKNOWN,n);
    uint8_t type=(uint8_t)(p[0]>>4), flags=(uint8_t)(p[0]&0x0fU);
    if (type==0U||type>15U) return out(OT_PARSE_UNPARSEABLE,OT_CLASS_UNKNOWN,n);
    uint8_t required_flags=(type==6U||type==8U||type==10U)?2U:0U;
    if ((type!=3U && flags!=required_flags) || (type==3U && ((flags>>1)&3U)==3U)) return out(OT_PARSE_UNPARSEABLE,OT_CLASS_UNKNOWN,n);
    uint32_t remaining=0; unsigned shift=0U, i=1U; uint8_t digit=0;
    do { if(i>=n || i>4U) return out(OT_PARSE_PARTIAL,OT_CLASS_UNKNOWN,n); digit=p[i++]; remaining+=(uint32_t)(digit&0x7fU)<<shift; shift+=7U; } while((digit&0x80U)!=0U);
    if (i==5U && (digit&0x80U)!=0U) return out(OT_PARSE_UNPARSEABLE,OT_CLASS_UNKNOWN,n);
    if ((size_t)remaining>n-i) return out(OT_PARSE_PARTIAL,OT_CLASS_UNKNOWN,n);
    ot_business_class_t cls=type==3U?OT_CLASS_MQTT_PUBLISH:OT_CLASS_MQTT_CONTROL;
    ot_decode_result_t r=out(OT_PARSE_OK,cls,n); r.payload_offset=i; r.payload_length=remaining;
    r.function_or_message_type=type; r.quality_of_service=(uint8_t)((p[0]>>1)&3U); r.transaction_flags=flags; r.declared_length=remaining;
    if (type==3U && remaining>=2U) { uint16_t topic=(uint16_t)(((uint16_t)p[i]<<8)|p[i+1U]); if ((uint32_t)topic+2U>remaining) r.state=OT_PARSE_UNPARSEABLE; else { r.protocol_id=topic; r.payload_offset=i+2U+topic; r.payload_length=(size_t)remaining-(2U+topic); if(r.quality_of_service && r.payload_length>=2U){ r.transaction_id=(uint16_t)(((uint16_t)p[r.payload_offset]<<8)|p[r.payload_offset+1U]); r.payload_offset+=2U; r.payload_length-=2U; } } }
    return r;
}
