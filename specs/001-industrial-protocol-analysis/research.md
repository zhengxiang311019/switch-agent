# Research: 工业协议通信分析

**Date**: 2026-10-23
**Scope**: Resolve protocol categories and passive-observation metric limits for the switch-hosted analyser. Online search for switch-specific capture documentation was unavailable; no vendor-specific capacity claims are made.

## Decision 1: Classify protocol traffic by observable protocol class and fields

**Decision**: For EtherNet/IP distinguish explicit messaging from implicit I/O where fields allow, and include CIP service/class/instance identifiers only when decoded. For PROFINET distinguish discovery/configuration/diagnostic traffic from cyclic real-time traffic where the frame type and fields allow. Maintain an explicit unknown, encrypted, truncated, or unsupported category.

**Rationale**: The protocols contain different traffic classes; a single protocol-wide label would not provide the requested business distribution. ODVA describes EtherNet/IP as carrying CIP over standard Ethernet and IP, with explicit and implicit messaging. PI describes PROFINET traffic classes and real-time communication; RT communication can be identified at Ethernet framing level. Classifying only visible fields avoids promising payload-level semantic decoding for private or encrypted data.

**Alternatives considered**: Count only packets by EtherType/transport protocol (too coarse for business distribution); decode every application payload (unsupported for encrypted, private, or incomplete traffic and beyond the v1 scope).

## Decision 2: Treat latency and jitter as observation-point measurements

**Decision**: Calculate transaction RTT only when both request and response are observable at the same capture point and have a reliable correlation key. For cyclic traffic, report observed arrival interval/deviation only when a cycle or sequence can be identified. Label the metric and source window in the UI. Do not claim absolute one-way network delay from one switch capture point.

**Rationale**: A passive copy records packets at the observation point. It does not independently provide synchronized timestamps at both communicating endpoints. Local capture timing supports elapsed time between visible related frames, not full endpoint-to-endpoint one-way delay.

**Alternatives considered**: Call all observed inter-arrival time “latency” (misleading); require synchronized endpoint clocks (not specified and not available from the described capture alone).

## Decision 3: Separate missing OT traffic from analyzer capture loss

**Decision**: Count capture input, capture-queue drops where available, parser rejects, and inferred protocol sequence gaps separately. A missing expected response is a timeout observation, not proof of a network packet drop. A sequence gap may be called inferred loss only if the relevant protocol sequence is visible and capture completeness is adequate; otherwise mark it indeterminate.

**Rationale**: Mirror oversubscription, CPU receive queue pressure, ACL scope, or parser limitations can make packets absent from the analyzer even when the network delivered them. Without independent evidence, these causes cannot be distinguished.

**Alternatives considered**: Treat each sequence gap or unanswered request as a confirmed network drop (overstates certainty); suppress all loss indicators (hides useful evidence).

## Decision 4: Report only checksum/error information visible to the platform

**Decision**: Verify per SKU whether errored frames and FCS metadata reach the CPU. Until verified, expose switch port error counters as separate hardware evidence and mark per-frame FCS validation unavailable. Independently validate higher-layer checksums only when the complete relevant bytes are available and checksum offload or truncation does not invalidate interpretation.

**Rationale**: Ethernet MACs commonly strip FCS and switches may discard bad-FCS frames before mirroring. A normal CPU packet copy therefore cannot be assumed to expose bad wire frames.

**Alternatives considered**: Claim full FCS visibility (unsupported without a specific ASIC/driver contract); claim zero CRC errors when no bad frame was copied (confuses invisibility with absence).

## Decision 5: Qualify performance per target SKU

**Decision**: Publish supported mirror throughput in packets/s and bytes/s for each target device using the actual ACL/CPU receive path under representative concurrent forwarding load. Measure analyzer drops, time-stamp precision, UI update delay, CPU/memory use, and forwarding/control-plane impact. Reject unsupported load as “observation incomplete” and preserve user traffic.

**Rationale**: CPU injection and mirror limits vary by ASIC, configuration, CPU, and SDK. No numeric rate is defensible until the target switch is known.

**Alternatives considered**: Assume line-rate capture (unsafe and hardware-dependent); choose an arbitrary universal rate (not meaningful across SKUs).

## Protocol references

- ODVA, [EtherNet/IP Technology Overview](https://www.odva.org/publication_download/ethernet-ip-technology-overview/)
- ODVA, [EtherNet/IP](https://www.odva.org/technology-standards/key-technologies/ethernet-ip/)
- ODVA, [Specifications](https://www.odva.org/subscriptions-services/specifications/)
- PROFIBUS & PROFINET International, [PROFINET Technology Description](https://www.profinet.com/profinet-explained/technology-description)
- PROFIBUS & PROFINET International, [PROFINET FAQ](https://www.profinet.com/community/faq)

## Items for implementation readiness

- Select the switch model(s), ASIC, CPU, OS/SDK version and ACL copy mode.
- Confirm whether CPU copies include direction, VLAN tags, full frame bytes, hardware timestamps and FCS/error metadata; record whether mirror-drop counters are available.
- Establish per-SKU packet/byte rate, number of monitored ports/flows, CPU and memory budgets, retention window and management UI refresh target through qualification.
- Agree with product owners whether PROFINET IRT and encrypted CIP Security traffic should be counted as opaque traffic only or excluded from v1 protocol recognition.

## Product reference review: PROFINET-INspektor NT

The user-requested [Indu-Sol PROFINET-INspektor NT product page](https://www.indu-sol.com.cn/index/product/view/id/33) describes PROFINET real-time analysis/evaluation, topology and asset management, port statistics, per-device jitter and refresh rate, self-learning thresholds after an acceptance test, acceptance reports, time-server synchronization, topology updates triggered by new nodes/controller access, quality coloring in topology, and MQTT alarm actions.

**Adopted**: passive topology and observed-asset inventory; per-port data where the switch exposes it; per-device/relation quality coloring; admin-controlled baseline windows with reviewable thresholds; diagnostic/acceptance reports; and MQTT alarm delivery with visible failure state.

**Adapted**: reference topology scans are implemented as updates from newly observed traffic and switch-provided metadata. Time-server synchronization reuses the target switch's configured time source. Threshold learning is allowed only on complete windows and does not activate an incomplete baseline.

**Not adopted**: active network scans/active diagnostics, because the product request specifies passive ACL-copied traffic observation and safe non-interference; electrical leakage-current measurement, because the target switch has no specified sensor for it. The referenced product page exposes screenshot links for switch counters and network-load graphs, but those image assets were not retrievable during research, so no further visual/detail claims rely on them.
# Protocol decoder references

On 2026-10-24, Wireshark dissectors were reviewed as protocol behavior/field references:

- EtherNet/IP encapsulation and CIP: [packet-enip.c](https://github.com/wireshark/wireshark/blob/master/epan/dissectors/packet-enip.c), [packet-cip.c](https://github.com/wireshark/wireshark/blob/master/epan/dissectors/packet-cip.c).
- PROFINET shared, DCP and RT: [packet-pn.c](https://github.com/wireshark/wireshark/blob/master/plugins/epan/profinet/packet-pn.c), [packet-pn-dcp.c](https://github.com/wireshark/wireshark/blob/master/plugins/epan/profinet/packet-pn-dcp.c), [packet-pn-rt.c](https://github.com/wireshark/wireshark/blob/master/plugins/epan/profinet/packet-pn-rt.c).
- Modbus TCP MBAP, function and exception fields: [packet-mbtcp.c](https://github.com/wireshark/wireshark/blob/master/epan/dissectors/packet-mbtcp.c).
- MQTT fixed header, message types, flags, and protocol versions: [packet-mqtt.c](https://github.com/wireshark/wireshark/blob/master/epan/dissectors/packet-mqtt.c).
- TCP sequence analysis, retransmission/out-of-order/gap diagnostics and segment reassembly behavior: [Wireshark packet-tcp.c](https://github.com/wireshark/wireshark/blob/master/epan/dissectors/packet-tcp.c), [TCP analysis guide](https://www.wireshark.org/docs/wsug_html/#ChAdvTCPAnalysis), and [TCP reassembly guide](https://www.wireshark.org/docs/wsug_html/#ChAdvReassemblySection).
- OPC UA binary protocol framing and transport parsing: [opcua.c](https://github.com/wireshark/wireshark/blob/master/plugins/epan/opcua/opcua.c), [opcua_transport_layer.c](https://github.com/wireshark/wireshark/blob/master/plugins/epan/opcua/opcua_transport_layer.c), and the [OPC UA plugin directory](https://github.com/wireshark/wireshark/tree/master/plugins/epan/opcua).

The analyzer uses an independent C implementation. No Wireshark source is copied or linked. Wireshark source files are distributed under GPL-2.0-or-later, so importing implementation code would require a separate license-compatibility review. The analyzer provides bounded TCP sequence tracking and stream reassembly for supported PDU framings; TLS/OPC UA decryption, IP-fragment reassembly, and unbounded capture/reassembly are not provided.
