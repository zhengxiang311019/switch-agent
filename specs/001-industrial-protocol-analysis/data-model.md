# Data Model: 工业协议通信分析

此模型描述分析结果及其可信状态，不要求特定数据库或序列化格式。字段是否可填取决于目标 ASIC 的镜像元数据和协议是否可解析。

## Entities

### Monitoring Scope

- `scope_id`: 本地监测范围标识。
- `ports`: 被监测面板端口集合。
- `protocols`: EtherNet/IP、PROFINET 或两者。
- `capture_status`: `disabled`、`ready`、`no_traffic`、`degraded`、`unavailable`。
- `capture_profile`: 目标 SKU 配置引用，含支持速率和资源限制。
- `retention_days`: 汇总指标和异常事件保存天数，默认 `7`，用户可配置为大于 `7` 的值。
- `updated_at`: 最近状态更新时间。

Rules: 只统计配置范围内且实际送达分析入口的流量。入口范围或接收状态不明时，相关统计标为不完整。

### Endpoint

- `endpoint_id`: 聚合端点标识。
- `mac_address`, `ip_addresses`: 被观察地址，可为空或多值。
- `role`: 可识别的控制器、设备、扫描器/客户端、未知等角色。
- `identity_source`: 产生身份判断的协议字段或地址信息。
- `confidence`: `confirmed`、`probable`、`unknown`。
- `first_seen`, `last_seen`: 观测时间。

Rules: 身份信息按观察到的事实记录；地址冲突或变化不得静默合并成确定设备身份。

### Communication Relation

- `relation_id`: 聚合通信关系标识。
- `protocol`: EtherNet/IP 或 PROFINET。
- `source_endpoint_id`, `destination_endpoint_id`: 源与目的端点。
- `transport_or_frame_class`: TCP/UDP/IP、PROFINET NRT/RT/IRT 或未知。
- `business_category`: 显式服务、隐式 I/O、循环数据、发现/配置、诊断/告警或未知类别。
- `protocol_identifiers`: 可见的 CIP service/class/connection ID、PROFINET frame ID/cycle sequence 等字段。
- `first_seen`, `last_seen`: 关系观测时间。
- `parse_status`: `complete`、`partial`、`encrypted`、`unsupported`、`malformed`。

Rules: 关系按可见字段确定；无法匹配的协议标识不阻断基本端点/流量统计。

### Traffic Aggregate

- `scope_id`, `relation_id`, `time_bucket`: 聚合维度。
- `packet_count`, `byte_count`: 对分析入口实际接收帧的计数。
- `protocol_packet_count`, `unknown_packet_count`: 分类计数。
- `capture_drop_count`: 可从接收路径获得的采集丢弃数。
- `completeness`: `complete`、`degraded`、`unknown`。

Rules: 字节计数口径（线缆帧长或复制帧长度）按 SKU 固定并在 UI 中标明。业务类别占比的分母只使用同一过滤器、时间桶和明确的计数口径。

### Quality Measurement

- `relation_id`, `window_start`, `window_end`: 关系及测量窗口。
- `rtt_samples`, `rtt_min`, `rtt_mean`, `rtt_max`, `rtt_percentiles`: 双向可关联事务的观测 RTT；字段按产品所选统计集开放。
- `jitter_value`, `jitter_definition`: 抖动值及口径（RTT 变化或周期报文到达间隔变化）。
- `refresh_period`, `refresh_rate`: 可识别周期通信的观察周期/频率；按设备或通信关系关联，并注明样本窗口。
- `expected_response_timeout_ms`: 使用的响应等待阈值。
- `timeout_count`: 超时等待后未见匹配响应的计数。
- `sequence_gap_count`: 可见序列信息显示的缺口计数。
- `checksum_error_count`: 捕获路径可见的校验失败数。
- `capture_completeness`, `confidence`: `complete`/`degraded`/`unknown` 以及 `confirmed`/`inferred`/`unavailable`。

Rules: 无双向关联样本时 RTT 为 unavailable；不以零表示 unavailable。序列缺口、响应超时不得单独标记为 confirmed network loss。FCS 不可见时 checksum 结果必须 unavailable。

### Anomaly Event

- `event_id`, `event_time`, `event_type`: 稳定标识、观测时间及超时、序列缺口、校验失败、采集退化、解析失败等类型。
- `scope_id`, `relation_id`, `endpoint_ids`: 关联范围和端点。
- `observed_value`, `threshold`, `count`: 观测值、判定阈值、累计次数。
- `evidence`: 支撑判定的可见字段/来源计数。
- `confidence`: `confirmed`、`inferred`、`indeterminate`、`unavailable`。
- `status`: `active`、`cleared`、`historical`。

Rules: UI 需同时呈现事件类型与可信状态；不可见的测量不生成“正常”事件。

### Observation Health

- `scope_id`, `observed_at`: 监测范围及状态时间。
- `input_packets`, `input_bytes`, `input_drop_count`: 捕获入口统计（若平台提供）。
- `parser_reject_count`, `truncated_count`: 解析能力问题计数。
- `resource_state`: `normal`、`near_limit`、`limited`、`unknown`。
- `timebase_state`: `stable`、`changed`、`unknown`。
- `counter_epoch`: 设备重启/计数器复位后的统计代次。
- `completeness`: `complete`、`degraded`、`unknown`。

Rules: 资源受限、时钟变化或计数器重置时切分统计窗口，并将受影响结论降级。

### OT Asset and Topology

- `asset_id`: 从可见协议字段或交换机信息生成的资产标识。
- `endpoint_ids`: 关联的通信端点。
- `observed_ports`: 交换机提供的入端口/邻接端口及其识别来源。
- `relation_ids`: 该资产参与的通信关系。
- `asset_type`: 可识别的控制器、I/O 设备、驱动、交换机或未知类别。
- `identity_confidence`: `confirmed`、`probable`、`unknown`。
- `first_seen`, `last_seen`, `topology_trigger`: 首次/最近观测和新节点、控制器访问变化等触发信息。
- `port_statistics`: 平台提供的运行状态、流量和错误计数器；每个字段带来源及可用性。
- `quality_state`: `healthy`、`warning`、`critical`、`incomplete`、`unknown`。

Rules: 拓扑只陈述观察到或交换机明示的数据；不可确认连接关系时保留未知边/端口，不触发主动网络扫描。拓扑颜色须配文本状态。

### Baseline Profile

- `baseline_id`, `scope_id`, `started_at`, `ended_at`: 基线及监测范围和窗口。
- `selected_metrics`: 参与基线的 RTT、周期间隔、抖动、流量等指标。
- `data_completeness`, `timebase_state`: 基线窗口的采集完整性和时间同步状态。
- `threshold_suggestions`: 指标、建议阈值、来源统计及可调整状态。
- `status`: `collecting`、`complete`、`incomplete`、`reviewed`、`active`。
- `reviewed_by`, `activated_at`: 管理员审核和启用记录。

Rules: 数据不完整或时间基准不稳定时，配置状态不得进入 `active`。建议阈值可追溯至窗口和指标样本。

### Diagnostic Report

- `report_id`, `scope_id`, `window_start`, `window_end`: 报告标识、范围和查询窗口。
- `asset_summary`, `topology_summary`: 资产及拓扑摘要。
- `quality_summary`, `anomaly_events`: 指标摘要和事件证据。
- `observation_health`: 捕获完整度、时间基准、解析限制和数据可信度。
- `generated_by`, `generated_at`: 生成者和时间。

Rules: 报告仅包含请求时间窗内可用数据，并保留 inferred/confirmed/unavailable 的可信状态。

### MQTT Notification Target

- `target_id`, `broker_address`, `topic`: 管理员设置的目标标识、代理地址和主题。
- `tls_enabled`, `broker_auth_state`, `credential_reference`: TLS 状态、代理身份认证状态，以及指向交换机安全凭据管理机制的引用；不得将凭据明文保存在普通配置或分析数据库中。
- `enabled`, `connection_state`: 启用及连接状态。
- `last_attempt_at`, `last_result`, `last_error`: 最近发送状态和错误摘要。
- `event_id`: 关联的本地异常事件。

Rules: MQTT 连接必须使用 TLS 并进行代理身份认证；凭据值不得在界面或日志中明文暴露。MQTT 失败不能删除或覆盖本地异常事件；消息至少携带事件类别、时间、关系/资产、可信状态和证据摘要。

## Relationships

```text
Monitoring Scope 1 ── * Endpoint
Monitoring Scope 1 ── * Communication Relation
Endpoint 1 ── * Communication Relation (source or destination)
Communication Relation 1 ── * Traffic Aggregate
Communication Relation 1 ── * Quality Measurement
Communication Relation 1 ── * Anomaly Event
Monitoring Scope 1 ── * Observation Health
Monitoring Scope 1 ── * OT Asset and Topology
OT Asset and Topology * ── * Communication Relation
Monitoring Scope 1 ── * Baseline Profile
Monitoring Scope 1 ── * Diagnostic Report
Anomaly Event 1 ── * MQTT Notification Target delivery attempt
```

## Lifecycle

- 关系和端点由首次可识别流量创建，持续流量更新 `last_seen`。
- 时间桶关闭后只追加聚合摘要；默认保存最近 7 天，用户可设置更长时间。设备的可用存储限制实际最大保存时长，设置和界面须一致。
- 采集退化/计数器复位会结束当前完整性代次；恢复后从新代次重新统计，不跨代次合并质量结论。
- 超时事件在阈值届满且未见匹配响应时生成；迟到响应可以补充关联信息，但不删除历史观测。
- 用户筛选只派生视图，不改变原始观测事件和样本可信状态。
