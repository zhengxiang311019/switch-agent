#ifndef OT_ANALYZER_TCP_ANALYSIS_H
#define OT_ANALYZER_TCP_ANALYSIS_H
#include "../protocols/protocols.h"
#include <stddef.h>
#include <stdint.h>

#define OT_TCP_FLOW_CAPACITY 32U
#define OT_TCP_OUTSTANDING_CAPACITY 32U
#define OT_TCP_REASSEMBLY_WINDOW 2048U
#define OT_TCP_MAX_APPLICATION_PDU 8192U
typedef enum ot_capture_coverage { OT_CAPTURE_COVERAGE_UNKNOWN=0, OT_CAPTURE_COVERAGE_PARTIAL=1, OT_CAPTURE_COVERAGE_BIDIRECTIONAL=2 } ot_capture_coverage_t;
typedef struct ot_tcp_outstanding { uint32_t start_sequence,end_sequence; uint64_t sent_ns; uint8_t used,retransmitted; } ot_tcp_outstanding_t;
typedef struct ot_tcp_flow_stats {
    uint64_t packets, payload_bytes, sequence_gaps, unconfirmed_gaps;
    uint64_t confirmed_capture_gaps, resolved_gaps, reassembly_overflows;
    uint64_t retransmission_candidates, out_of_order_candidates, duplicate_acks;
    uint64_t acked_bytes, ack_rtt_count, ack_rtt_sum_ns, ack_timeout_candidates;
    uint64_t gap_timeout_candidates, tcp_option_errors, sack_blocks_seen, timestamp_options_seen;
    uint32_t next_expected_sequence, highest_sequence, last_acknowledgment, initial_sequence, bytes_in_flight;
    uint32_t last_window;
    uint32_t connection_generation;
    uint8_t sequence_initialized, syn_seen, fin_seen, rst_seen, coverage;
    uint8_t gap_active, application_protocol, application_parse_state, application_business_class;
    uint64_t application_pdu_count;
    uint64_t last_seen_ns;
    uint64_t gap_started_ns;
    uint8_t gap_timeout_reported;
} ot_tcp_flow_stats_t;
typedef struct ot_tcp_flow_entry {
    uint8_t src_ip[16],dst_ip[16],ip_version,vlan_count,used;
    uint16_t src_port,dst_port,vlan_id[OT_PACKET_MAX_VLAN_TAGS];
    ot_tcp_flow_stats_t stats;
    ot_tcp_outstanding_t outstanding[OT_TCP_OUTSTANDING_CAPACITY];
    uint8_t stream_bytes[OT_TCP_REASSEMBLY_WINDOW], stream_present[OT_TCP_REASSEMBLY_WINDOW], application_bytes[OT_TCP_MAX_APPLICATION_PDU];
    size_t application_length;
} ot_tcp_flow_entry_t;
typedef struct ot_tcp_flow_snapshot {
    uint8_t src_ip[16],dst_ip[16],ip_version,vlan_count;
    uint16_t src_port,dst_port,vlan_id[OT_PACKET_MAX_VLAN_TAGS];
    ot_tcp_flow_stats_t stats;
} ot_tcp_flow_snapshot_t;
typedef struct ot_tcp_tracker { ot_tcp_flow_entry_t flows[OT_TCP_FLOW_CAPACITY]; size_t count; uint64_t rto_ns,flow_capacity_drops; } ot_tcp_tracker_t;
void ot_tcp_tracker_init(ot_tcp_tracker_t *tracker,uint64_t rto_ns);
int ot_tcp_tracker_ingest(ot_tcp_tracker_t *tracker,const ot_packet_view_t *packet,const uint8_t *frame,uint64_t timestamp_ns,ot_capture_coverage_t coverage);
void ot_tcp_tracker_expire(ot_tcp_tracker_t *tracker,uint64_t now_ns);
const ot_tcp_flow_entry_t *ot_tcp_tracker_find(const ot_tcp_tracker_t *tracker,const ot_packet_view_t *packet);
#endif
