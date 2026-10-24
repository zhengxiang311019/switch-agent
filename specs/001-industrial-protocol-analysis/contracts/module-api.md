# Module API Contract

## Platform input (HAL)

- `ot_hal_capture_t.receive(context, timeout_ms, packet)` supplies one copied-frame view and returns OK, TIMEOUT, UNAVAILABLE, or ERROR.
- Frame bytes are borrowed until the next receive callback. The module validates lengths and copies accepted data to bounded storage.
- `metadata_flags` defines availability for port, direction, timestamp, checksum, and source counters. Absence is unavailable, never an inferred zero.
- `get_counters` is optional. The host reports its hardware counter reset epoch through `ot_timebase_reset_epoch` when known.

## Host output/control API

- `ot_module_create` allocates an opaque module instance configured with scope, bucket width, and retention ceiling; `ot_module_destroy` clears and releases it.
- `ot_module_poll` receives via HAL and analyzes one record; `ot_module_ingest` accepts a host-owned packet view for replay/integration.
- `ot_module_get_overview` returns typed counters and explicit disabled/no-traffic/ready/degraded/unavailable state.
- `ot_module_set_capture_coverage` accepts the host's configured observation coverage (unknown, partial/asymmetric, or complete bidirectional path); default is unknown.
- `ot_module_set_tcp_rto` sets the bounded diagnostic timeout threshold; the initial default is 1 second and host monotonic timestamps are required for timeout/RTT calculations.
- `ot_module_tick` advances monotonic TCP ACK/gap timeout diagnostics when no packet arrives.
- `ot_module_get_tcp_diagnostics` copies bounded TCP flow snapshots containing VLAN/IP/ports, sequence/ACK indicators, timeout/gap/retransmission candidates, reassembly status, and application classification. Snapshots exclude stream reassembly bytes.
- `ot_module_get_tcp_summary` reports active directional-flow entries, the count of flows rejected because the bounded tracker could not safely evict an entry, and the configured RTO. Hosts can surface capacity pressure instead of interpreting an incomplete flow list as complete. A null output pointer is ignored and produces no write.
- `ot_module_set_retention` accepts the host's authorization decision and rejects non-admin changes.
- `ot_protocol_decode` is the public frame-decoder entry point for supported EtherNet/IP, PROFINET, Modbus TCP, OPC UA TCP, and MQTT frames. Its typed result carries protocol, business class, parse state, bounded field identifiers, and payload offsets/lengths; it never owns or persists input bytes.
- UI, MQTT transport/TLS, credential storage, configuration persistence, and switch account authentication are host responsibilities. Analyzer outputs provide typed data and notification events without linking those services.

The host-facing boundary is `module_api.h`; the lower-layer capture boundary is `platform/hal_capture.h`. The analyzer neither invokes the host UI/network services nor accesses switch hardware directly. The host supplies HAL/authorization/service adapters and consumes typed snapshots/events. The module implementation is opaque so transient stream-reassembly buffers are not exposed through the host API.
