#include "protocols.h"
#include <string.h>

static uint32_t le32(const uint8_t *p) { return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24); }
static ot_decode_result_t out(ot_parse_state_t s, ot_business_class_t c, size_t n) {
    ot_decode_result_t r; memset(&r,0,sizeof(r)); r.protocol=OT_PROTO_OPC_UA; r.state=s;
    r.business_class=c; r.payload_offset=8U; r.payload_length=n>8U?n-8U:0U; return r;
}
ot_decode_result_t ot_decode_opcua(const uint8_t *p, size_t n) {
    if (!p||n<8U) return out(OT_PARSE_PARTIAL,OT_CLASS_UNKNOWN,n);
    uint32_t size=le32(p+4);
    if (size<8U) return out(OT_PARSE_UNPARSEABLE,OT_CLASS_UNKNOWN,n);
    if ((size_t)size>n) return out(OT_PARSE_PARTIAL,OT_CLASS_UNKNOWN,n);
    int control = memcmp(p,"HEL",3)==0 || memcmp(p,"ACK",3)==0 || memcmp(p,"ERR",3)==0 || memcmp(p,"RHE",3)==0;
    if (!control && memcmp(p,"OPN",3)!=0 && memcmp(p,"MSG",3)!=0 && memcmp(p,"CLO",3)!=0)
        return out(OT_PARSE_UNPARSEABLE,OT_CLASS_UNKNOWN,n);
    ot_decode_result_t r=out(OT_PARSE_OK,control?OT_CLASS_OPCUA_HEL:OT_CLASS_OPCUA_SECURE,n); r.declared_length=size;
    r.transaction_flags=p[3];
    /* OPN/MSG/CLO secure-channel bodies may be signed or encrypted. Keep headers and sizes only. */
    return r;
}
