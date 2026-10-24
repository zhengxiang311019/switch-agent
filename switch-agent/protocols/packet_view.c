#include "protocols.h"
#include <string.h>

static uint16_t be16v(const uint8_t *p) { return (uint16_t)(((uint16_t)p[0] << 8) | p[1]); }
static uint32_t be32v(const uint8_t *p) { return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3]; }

/* Extracts transport metadata without assuming a particular OT protocol. */
int ot_packet_view_decode(const uint8_t *f, size_t n, ot_packet_view_t *v)
{
    if (!f || !v || n < 14U) return 0;
    memset(v, 0, sizeof(*v)); memcpy(v->dst_mac,f,6); memcpy(v->src_mac,f+6,6);
    size_t off=14U; uint16_t type=be16v(f+12U);
    while ((type==0x8100U || type==0x88a8U || type==0x9100U) && v->vlan_count<OT_PACKET_MAX_VLAN_TAGS) {
        if (n-off<4U) return 0;
        v->vlan_id[v->vlan_count++]=(uint16_t)(be16v(f+off)&0x0fffU);
        type=be16v(f+off+2U); off+=4U;
    }
    v->l3_offset=off;
    if (type==0x0800U) {
        if(n-off<20U || (f[off]>>4)!=4U) return 0;
        size_t ihl=(size_t)(f[off]&0x0fU)*4U; if(ihl<20U || n-off<ihl) return 0;
        uint16_t total=be16v(f+off+2U); if(total<ihl) return 0;
        if(n-off>total) n=off+total;
        uint16_t frag=be16v(f+off+6U); v->fragmented=(uint8_t)((frag&0x3fffU)!=0U);
        v->ip_version=4U; memcpy(v->src_ip,f+off+12U,4); memcpy(v->dst_ip,f+off+16U,4);
        v->transport_protocol=f[off+9U]; off+=ihl;
        if ((frag&0x1fffU)!=0U) { v->l4_offset=off; v->payload_offset=off; return 1; }
    } else if (type==0x86ddU) {
        if(n-off<40U || (f[off]>>4)!=6U) return 0;
        uint16_t plen=be16v(f+off+4U); if(n-off>40U+(size_t)plen) n=off+40U+(size_t)plen;
        v->ip_version=6U; memcpy(v->src_ip,f+off+8U,16); memcpy(v->dst_ip,f+off+24U,16);
        uint8_t next=f[off+6U]; off+=40U;
        for(unsigned ext=0;ext<8U;ext++) {
            if(next==0U || next==43U || next==60U) { if(n-off<2U)return 0; size_t el=((size_t)f[off+1U]+1U)*8U; if(n-off<el)return 0; next=f[off]; off+=el; }
            else if(next==44U) { if(n-off<8U)return 0; uint16_t frag=be16v(f+off+2U); v->fragmented=(uint8_t)((frag&0xfff9U)!=0U); next=f[off]; off+=8U; if((frag&0xfff8U)!=0U){v->transport_protocol=next;v->l4_offset=off;v->payload_offset=off;return 1;} }
            else if(next==51U) { if(n-off<2U)return 0; size_t el=((size_t)f[off+1U]+2U)*4U; if(n-off<el)return 0; next=f[off]; off+=el; }
            else break;
        }
        v->transport_protocol=next;
    } else return 1;
    v->l4_offset=off;
    if(v->transport_protocol==6U) {
        if(n-off<20U)return 0;
        size_t hlen=(size_t)(f[off+12U]>>4)*4U; if(hlen<20U||n-off<hlen)return 0;
        v->src_port=be16v(f+off);v->dst_port=be16v(f+off+2U);v->tcp_sequence=be32v(f+off+4U);v->tcp_acknowledgment=be32v(f+off+8U);v->tcp_flags=f[off+13U];v->tcp_window=be16v(f+off+14U);v->tcp_options_valid=1U;
        for(size_t q=off+20U;q<off+hlen;){uint8_t kind=f[q];if(kind==0U)break;if(kind==1U){q++;continue;}if(q+2U>off+hlen){v->tcp_options_valid=0U;break;}uint8_t olen=f[q+1U];if(olen<2U||q+olen>off+hlen){v->tcp_options_valid=0U;break;}if(kind==2U&&olen==4U)v->tcp_mss=be16v(f+q+2U);else if(kind==3U&&olen==3U)v->tcp_window_scale=f[q+2U];else if(kind==4U&&olen==2U)v->tcp_sack_permitted=1U;else if(kind==5U&&olen>=10U)v->tcp_sack_blocks=(uint8_t)((olen-2U)/8U);else if(kind==8U&&olen==10U){v->tcp_timestamp_value=be32v(f+q+2U);v->tcp_timestamp_echo=be32v(f+q+6U);}q+=olen;}
        off+=hlen;
    } else if(v->transport_protocol==17U) {
        if(n-off<8U)return 0;
        uint16_t ulen=be16v(f+off+4U); if(ulen<8U)return 0; if(n-off>ulen)n=off+ulen;
        v->src_port=be16v(f+off);v->dst_port=be16v(f+off+2U);off+=8U;
    } else { v->payload_offset=off; v->payload_length=n-off; return 1; }
    v->payload_offset=off;v->payload_length=n-off;return 1;
}
