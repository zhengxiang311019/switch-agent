#include "protocols.h"

#include <string.h>

ot_decode_result_t ot_decode_profinet(const uint8_t *data, size_t length,
                                      size_t frame_offset)
{
    ot_decode_result_t r;
    memset(&r, 0, sizeof(r));
    r.protocol = OT_PROTO_PROFINET;
    r.payload_offset = frame_offset;
    r.payload_length = length;
    if (data == NULL || length < 2U) { r.state=OT_PARSE_UNPARSEABLE; r.business_class=OT_CLASS_UNKNOWN; return r; }
    r.service_or_frame_id=(uint16_t)(((uint16_t)data[0]<<8)|data[1]);
    r.state=OT_PARSE_OK;
    if (r.service_or_frame_id >= 0xfefcU && r.service_or_frame_id <= 0xfeffU) {
        r.business_class=OT_CLASS_PN_DCP;
        if (length < 12U) { r.state=OT_PARSE_PARTIAL; return r; }
        r.pn_dcp_service=data[2];
        r.pn_dcp_xid=((uint32_t)data[4]<<24)|((uint32_t)data[5]<<16)|((uint32_t)data[6]<<8)|data[7];
        r.pn_dcp_data_length=(uint16_t)(((uint16_t)data[10]<<8)|data[11]);
        if ((size_t)r.pn_dcp_data_length > length-12U) { r.state=OT_PARSE_PARTIAL; return r; }
        size_t pos=12U, end=12U+r.pn_dcp_data_length;
        while(pos<end){
            if(end-pos<4U){r.state=OT_PARSE_PARTIAL;break;}
            uint16_t block_length=(uint16_t)(((uint16_t)data[pos+2U]<<8)|data[pos+3U]);
            pos+=4U;
            if((size_t)block_length>end-pos){r.state=OT_PARSE_PARTIAL;break;}
            r.pn_dcp_blocks++; pos+=(size_t)block_length;
            if((block_length&1U)!=0U&&pos<end)pos++;
        }
    }
    else if (r.service_or_frame_id >= 0xfc01U && r.service_or_frame_id <= 0xfcffU) r.business_class=OT_CLASS_PN_ALARM;
    else if (r.service_or_frame_id >= 0x0100U && r.service_or_frame_id < 0xf000U) r.business_class=OT_CLASS_PN_REALTIME;
    else r.business_class=OT_CLASS_UNKNOWN;
    if (length < 4U) r.state=OT_PARSE_PARTIAL;
    return r;
}

