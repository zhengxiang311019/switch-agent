# OT Observability Contract

这是面向设备现有管理界面的用户可见契约，以及分析器与交换机复制流量源之间的逻辑边界。具体 API、CLI、UI 框架及传输协议由目标产品既有管理栈决定，本文件不引入新的外部 API。

## User-visible views

| View | Required inputs | Required output | Empty/degraded behavior |
|---|---|---|---|
| OT Overview | time range, optional protocol/port | per-protocol packet/byte rate, endpoints, relations, observation health | distinguish no matching traffic from unavailable/incomplete capture |
| Relation Detail | relation, time range | business class, traffic trend, available RTT/jitter, timeout/gap/checksum counts, evidence and metric definitions | unavailable metrics show a reason; never render unknown as zero/normal |
| Business Distribution | time range, protocol, optional endpoint/relation | category packet/byte totals and share, unknown/encrypted/partial categories | use a consistent denominator and include unknown traffic |
| Event Detail | event or filtered event list | type/time, relation/endpoints, value/threshold, evidence, confidence, capture health | identify inferred vs confirmed observations |
| Monitoring Status | configured scope | monitored ports/protocols, input and resource state, last update, completeness | explain disabled, no traffic, degraded, or unavailable |
| Retention Settings | current retention duration | default 7 days and user-selected longer duration | display the active value and device-supported maximum |
| OT Topology and Assets | observation scope and time window | observed devices, ports, relations, per-relation jitter, quality state, identity confidence | unknown nodes/edges stay explicitly unknown; no active scan; missing jitter stays unavailable |
| Baseline Review | selected scope, metrics, observation window | data completeness, threshold suggestions, source samples, review/apply controls | incomplete or time-unstable windows cannot be activated |
| Diagnostic Report | selected scope and time window | asset/topology summary, quality metrics, event evidence, observation health | label unavailable fields and preserve confidence states |
| MQTT Alerts | enabled target and event rules; TLS and broker-auth configuration | authenticated event summary delivery state | require TLS and broker authentication; protect credentials from UI/log exposure; keep local event and surface connection, authentication, or delivery failure |

## Filter and aggregation semantics

- Filters apply consistently to counts, rates, trends, category shares, and event lists.
- Each result declares its observation interval, protocol classification, and completeness/confidence state.
- Rates and shares use the same time window and byte-count convention; unknown traffic stays in the denominator when showing capture-wide distribution.
- An inferred sequence gap or unobserved response is not surfaced as confirmed network loss.
- Per-frame checksum/FCS status is unavailable unless the target receive path explicitly supplies the relevant data. Port-level hardware error counters remain a separate source.
- Raw application payload is not displayed or retained by default.
- Topology is inferred from passive traffic and switch-supplied port data; no active scan is sent to OT devices.
- All timestamps use the switch-configured time source; unstable synchronization is surfaced as degraded evidence.
- Users with view permission can inspect results and current settings; only administrators can change monitoring and retention settings, using the switch's existing account and permission system.

## HAL capture boundary

The analyzer consumes packet records only through a stable, platform-neutral HAL receive contract. Each record supplies copied frame bytes, ingress direction/port metadata when available, and an observation timestamp; optional hardware error/drop metadata and source counters are accompanied by explicit availability. The HAL reports source counters and resource state independently from protocol results. Missing metadata is represented as unavailable; the analyzer must not synthesize it. ASIC, CPU, OS, SDK, ACL programming, and driver details belong to the HAL implementation and are not visible to analyzer modules.
