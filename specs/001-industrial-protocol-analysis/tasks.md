---
description: "Dependency-ordered implementation tasks for industrial protocol analysis"
---

# Tasks: 工业协议通信分析

**Input**: Design documents from `specs/001-industrial-protocol-analysis/`

**Prerequisites**: `plan.md`, `spec.md`, `research.md`, `data-model.md`, `contracts/`, `quickstart.md`

**Tests**: No implementation test tasks are included because the feature specification did not request tests or a TDD workflow. Use `quickstart.md` as the validation guide when the target device and SDK are available.

**Organization**: Tasks are grouped by user story; P1 stories are ordered before P2.

**Path convention**: `switch-agent/` and `qualification/` are planned logical roots from `plan.md`; map them into the selected switch product repository and SDK before implementation.

## Phase 1: Setup

**Purpose**: Define the stable HAL boundary and prepare the platform-neutral analyzer module layout.

- [X] T001 Define the platform-neutral HAL receive contract for packet bytes, ingress metadata, timestamps, optional hardware counters/error metadata, availability, and receive status in `switch-agent/platform/hal_capture`.
- [X] T002 Create the analyzer module skeleton with a HAL interface boundary; ensure protocol, analysis, storage, reporting, notification, and presentation modules have no direct vendor SDK or hardware dependencies.
- [X] T003 [P] Add a protocol classification and unsupported-traffic category registry in `switch-agent/protocols/category_registry`.

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Implement shared capture, time, resource, and identity foundations required by all user stories.

- [X] T004 Implement a HAL-backed capture consumer that receives packet records only via the stable HAL contract and exposes optional metadata/availability without vendor-specific calls in `switch-agent/platform/hal_capture`.
- [X] T005 Implement bounded receive queues and separate input-drop, truncation, and resource-pressure counters in `switch-agent/capture/health` (depends on T004).
- [X] T006 [P] Implement device timebase and counter-epoch tracking, including reset/discontinuity status, in `switch-agent/platform/timebase`.
- [X] T007 Implement shared Monitoring Scope, Endpoint, and Communication Relation records with the enumerated status/confidence values from `data-model.md` in `switch-agent/analysis/flow_catalog`.
- [X] T008 Implement common frame-length accounting and time-bucket aggregation with a single documented byte-count convention in `switch-agent/analysis/traffic_aggregate` (depends on T006).
- [X] T009 Implement protocol-neutral observation completeness and confidence propagation in `switch-agent/analysis/observation_health` (depends on T005, T006).
- [X] T010 Expose shared observation status and OT overview snapshot data through a platform-neutral presentation API in `switch-agent/presentation/monitoring_status` (host UI binding remains product integration).

**Checkpoint**: HAL capture state, flow identity, basic aggregation, and data completeness are available to each story; the analyzer remains independent of target hardware and SDK.

---

## Phase 3: User Story 1 - 查看 OT 通信健康状况 (Priority: P1) 🎯 MVP

**Goal**: Let an industrial network engineer see protocol, endpoint, relation, traffic, and current observation health.

**Independent Test**: Use a known trace containing all supported protocols; verify the overview shows protocol counts, endpoints, relations, and monitoring status, and shows “no matching traffic” distinctly from unavailable or degraded capture.

### Implementation for User Story 1

- [X] T011 [P] [US1] Recognize EtherNet/IP encapsulation and classify visible explicit/implicit traffic in `switch-agent/protocols/ethernet_ip`.
- [X] T012 [P] [US1] Recognize visible PROFINET discovery/configuration, diagnostic, and real-time frame classes in `switch-agent/protocols/profinet`.
- [X] T013 [US1] Populate bounded communication-relation records from supported MAC/IP/transport and protocol fields, retaining partial/unknown parse status in `switch-agent/analysis/flow_catalog` (depends on T007, T011, T012).
- [X] T014 [US1] Aggregate recognized and unknown packet/byte counts by configured time bucket in `switch-agent/analysis/traffic_aggregate` (depends on T008, T013).
- [X] T015 [US1] Present OT overview snapshot data with no-traffic and degraded/unavailable capture states in `switch-agent/presentation/monitoring_status` (depends on T009, T014).
- [X] T016 [US1] Add retention policy defaults and administrator-only duration updates through a host-supplied permission decision in `switch-agent/store/retention` (depends on T007, T008).

**Checkpoint**: The P1 monitoring overview works independently using available traffic and capture-health data.

---

## Phase 4: User Story 2 - 分析通信时延与异常 (Priority: P1)

**Goal**: Let an engineer inspect observable RTT/jitter and timeout, sequence-gap, and checksum evidence without overstating certainty.

**Independent Test**: Use bidirectional correlatable and one-way traces plus known response timeouts, sequence gaps, and platform-exposed checksum evidence; verify available metrics match the reference, unsupported metrics show why they are unavailable, and capture loss is not called confirmed network loss.

### Implementation for User Story 2

- [X] T017 [P] [US2] Add bounded keyed transaction correlation storage for supported bidirectional exchanges in `switch-agent/analysis/quality`.
- [X] T018 [US2] Compute RTT count, sum, minimum, maximum, and last sample only for correlatable bidirectional exchanges in `switch-agent/analysis/quality` (depends on T017).
- [ ] T019 [US2] Compute explicitly labeled RTT variation or cyclic inter-arrival jitter from valid samples in `switch-agent/analysis/jitter_metrics` (depends on T017).
- [X] T020 [US2] Implement configurable response-wait threshold handling and distinguish timeout observations from inferred sequence gaps in `switch-agent/analysis/quality`.
- [X] T021 [P] [US2] Consume only caller-confirmed visible per-frame checksum/FCS metadata and count port-level errors separately in `switch-agent/analysis/quality`.
- [ ] T022 [US2] Create anomaly events with evidence, threshold, capture completeness, confidence, and active/cleared/historical status in `switch-agent/analysis/anomaly_events` (depends on T009, T018, T020, T021).
- [ ] T023 [US2] Present relation quality trends, metric definitions, threshold, event evidence, and unavailable reasons in `switch-agent/presentation/relation_quality` (depends on T018, T019, T022).

**Checkpoint**: RTT, jitter, timeout, gap, and checksum views make observation limits and confidence visible.

---

## Phase 5: User Story 3 - 了解业务类型和流量分布 (Priority: P2)

**Goal**: Let an OT engineer compare recognized business/service classes and their traffic shares while retaining unknown traffic.

**Independent Test**: Use a trace with multiple identifiable service categories across supported protocols and unknown, encrypted, or truncated frames; verify category packet/byte totals and shares use the selected filters and unknown traffic remains separate.

### Implementation for User Story 3

- [X] T024 [P] [US3] Extract visible EtherNet/IP encapsulation command and CIP service identifiers without retaining application payload in `switch-agent/protocols/ethernet_ip`.
- [X] T025 [P] [US3] Extract PROFINET frame IDs and bounded DCP service/XID/TLV block fields in `switch-agent/protocols/profinet`.
- [ ] T026 [US3] Aggregate recognized and unknown business categories, packet/byte totals, and consistent-denominator shares in `switch-agent/analysis/business_distribution` (depends on T008, T024, T025).
- [ ] T027 [US3] Present business distribution with time, protocol, endpoint, and relation filters and explicit unknown/encrypted/unparseable categories in `switch-agent/presentation/business_distribution` (depends on T026).

**Checkpoint**: Business distribution is independently useful and never folds unknown traffic into a recognized category.

---

## Phase 6: User Story 4 - 查看 OT 拓扑和资产状态 (Priority: P2)

**Goal**: Show a passive evidence-based asset/topology view with quality and available port context.

**Independent Test**: Replay traffic with known and unknown devices, relations, and port metadata; confirm observed assets/edges and available port counters appear with confidence and quality states, without active scans.

### Implementation for User Story 4

- [ ] T028 [P] [US4] Define observed OT asset/topology records with identity confidence, relation links, first/last seen, trigger and quality state in `switch-agent/analysis/asset_topology`.
- [ ] T029 [US4] Update asset/topology records from newly observed nodes and controller-access changes using passive packet evidence in `switch-agent/analysis/topology_updates` (depends on T004, T011, T012, T028).
- [ ] T030 [P] [US4] Collect target-platform port status and available traffic/error counters with source and availability metadata in `switch-agent/platform/port_stats`.
- [ ] T031 [US4] Present passive topology/assets, quality states, identity confidence, available per-port statistics, per-device relation jitter, and PROFINET refresh period/rate in `switch-agent/presentation/topology_assets` (depends on T009, T019, T022, T029, T030).

**Checkpoint**: Engineers can navigate from an observed quality issue to the relevant asset, relation, and available switch port evidence.

---

## Phase 7: User Story 5 - 建立通信基线并分发诊断结果 (Priority: P2)

**Goal**: Let administrators generate reviewable quality thresholds, diagnostic reports, and MQTT alarm summaries.

**Independent Test**: Use complete and degraded baseline windows; verify threshold suggestions are reviewable and incomplete data is not activated, generated reports include required context, and MQTT delivery status is visible while failed alerts remain locally available.

### Implementation for User Story 5

- [ ] T032 [US5] Surface configured time-source and synchronization state for metrics, events, and reports in `switch-agent/presentation/time_sync_status` (depends on T006, T009).
- [ ] T033 [US5] Build a metric baseline from an administrator-selected window and generate traceable threshold suggestions only when capture completeness and timebase are valid in `switch-agent/analysis/baseline_thresholds` (depends on T009, T018, T019, T032).
- [ ] T034 [US5] Let administrators review, adjust, and activate baseline threshold suggestions in `switch-agent/presentation/baseline_review` (depends on T033).
- [ ] T035 [US5] Generate and present/export acceptance and diagnostic reports with scope, window, asset/topology summary, quality, events and observation health in `switch-agent/reporting/diagnostic_report` (depends on T022, T029, T031, T033).
- [ ] T036 [US5] Configure TLS-encrypted MQTT alarm delivery with broker authentication, retrieve credentials through the switch secure credential facility without exposing them in UI/logs, and show connection/authentication/publish failures while retaining the local event in `switch-agent/notifications/mqtt_alerts` (depends on T001 and T022).

**Checkpoint**: Complete baselines, reports, and MQTT notifications preserve evidence, timebase, and confidence information; MQTT requires TLS and broker authentication and protects credentials.

---

## Phase 8: Polish & Cross-Cutting Concerns

**Purpose**: Document HAL integration requirements and qualify resource limits without coupling the analyzer to a device SKU.

- [X] T037 Define the HAL conformance and capability-availability expectations, including packet-rate behavior, timestamps, capture-drop reporting, error counters, and resource-state reporting, without adding SKU-specific branches to analyzer logic.
- [X] T038 Document how a platform HAL implementation is selected and integrated with the analyzer, including the HAL contract and how unsupported metadata is represented, in `specs/001-industrial-protocol-analysis/quickstart.md`.
- [X] T039 [P] Add bounded Modbus TCP MBAP/function, OPC UA TCP chunk-header, and MQTT fixed-header/remaining-length decoders with explicit partial/unparseable/encrypted classification in `switch-agent/protocols/`.
- [X] T040 Update protocol scope, business categories, Wireshark reference/provenance, and frame-only decoding limitations in `spec.md`, `plan.md`, `protocols/README.md`, and `quickstart.md`.
- [X] T041 Add bounded per-direction TCP stream reassembly for split/coalesced protocol messages, restore in-order delivery after limited out-of-order buffering, and carry protocol decode metadata into analysis aggregates without exposing or persisting payloads.
- [X] T042 [P] Decode up to four VLAN tags, IPv4/IPv6 and TCP/UDP header fields (sequence/ACK, flags, window and common options), while bounding IPv6 extension parsing and marking fragments in `switch-agent/protocols/packet_view`.
- [X] T043 Track direction-specific TCP gaps, retransmission/overlap and out-of-order candidates, duplicate ACKs, ACK RTT/timeout candidates, connection reuse, and reassembly overflow in `switch-agent/analysis/tcp_analysis`.
- [X] T044 Add host-declared capture coverage and typed TCP diagnostics snapshot APIs; classify gaps as unconfirmed when mirrored path coverage is unknown/partial in `switch-agent/module_api`.
- [X] T045 Expose bounded TCP tracker occupancy, rejected-flow count, and RTO summary; resolve gap state when contiguous FIN closes the outstanding sequence range.

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: T001 establishes the stable HAL receive contract; T002/T003 establish the platform-neutral layout and protocol categories.
- **Foundational (Phase 2)**: Depends on the HAL contract and blocks story implementation. T006 can proceed in parallel with T004/T005; T007 depends on capture records; T008/T009 depend on shared data/time foundations; T010 depends on T009.
- **User Stories (Phase 3+)**: Start after Phase 2. US1 and US2 are P1; US3, US4, and US5 are P2. US4 depends on protocol recognition and port metadata; US5 depends on quality metrics, observation health, and timebase. Story-specific task dependencies are noted inline.
- **Polish (Phase 8)**: Finalize each SKU profile after platform and management integration choices are known.

### User Story Dependencies

- **US1 (P1)**: Depends only on foundational capture, flow catalog, aggregation, and health components; no dependency on US2 or US3. T016 depends on the shared aggregation and configuration surface.
- **US2 (P1)**: Depends on foundational capture/time and relation records plus base protocol recognition from T011/T012; does not require the completed US1 overview.
- **US3 (P2)**: Depends on foundational aggregation and base protocol recognition from T011/T012; does not require US2 metrics.
- **US4 (P2)**: Depends on platform port metadata and base protocol recognition; it may share quality-state output from US2 where available.
- **US5 (P2)**: Depends on quality metrics/events and stable timebase; reports include topology context when US4 data is available.

### Parallel Opportunities

- After T001, T002 and T003 can proceed in parallel; T003 creates the category registry independent of the module skeleton.
- In Phase 2, T006 can proceed alongside T004/T005; T007/T008/T009/T010 follow their stated prerequisites.
- In US1, T011 and T012 are independent protocol modules; T015 follows aggregation and status wiring.
- In US2, T017 and T021 can proceed independently; UI presentation follows metric/event tasks.
- In US3, T024 and T025 are independent protocol modules; category aggregation and presentation follow both.
- In US4, T028 and T030 are independent model/platform tasks after Phase 2; T031 follows passive topology updates and port metadata.
- In US5, time-sync status (T032) and MQTT event delivery (T036) can proceed independently once their listed prerequisites are met; reporting follows asset/topology context.
- After Phase 2, start US1. Once T011/T012 land, US2 and US3 can proceed separately from the remainder of US1, subject to inline shared module dependencies.

### Parallel Execution Examples

```text
US1 after Phase 2: T011 EtherNet/IP recognition || T012 PROFINET recognition
US2 after T011/T012: T017 transaction correlation || T021 checksum visibility handling
US3 after T011/T012: T024 EtherNet/IP business fields || T025 PROFINET business fields
US4 after Phase 2: T028 asset/topology record || T030 switch port statistics
US5 after prerequisites: T032 time-sync state || T036 MQTT delivery
```

## Implementation Strategy

### MVP First (User Story 1)

1. Complete Setup and Foundational phases, including target-SKU profile selection.
2. Complete US1 to deliver protocol traffic, endpoint/relation overview, and capture-health states.
3. Validate US1 independently using the known-trace scenario in `quickstart.md` before extending the UI.

### Incremental Delivery

1. Deliver US1 overview first as the smallest useful product increment.
2. Add US2 communication-quality metrics and evidence/confidence semantics.
3. Add US3 business/service distribution.
4. Add US4 topology/asset context and US5 baseline, report, and notification workflows.
5. Complete the per-SKU workload and resource qualification before declaring supported operating limits.
