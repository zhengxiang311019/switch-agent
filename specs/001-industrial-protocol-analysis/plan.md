# Implementation Plan: 工业协议通信分析

**Branch**: `001-industrial-protocol-analysis` | **Date**: 2026-10-23 | **Spec**: [spec.md](spec.md)

**Input**: Feature specification from `specs/001-industrial-protocol-analysis/spec.md`

## Summary

在交换机 CPU 上对 ACL 复制的面板口流量进行被动 EtherNet/IP、PROFINET、Modbus TCP、OPC UA TCP 和 MQTT 解析，将采集完整性、协议业务分类、通信质量样本、资产/拓扑及异常事件分层汇总，并通过设备现有管理界面呈现。设计将镜像观测结果与网络真实丢包分开：RTT 只对可关联的双向事务计算，周期业务抖动基于可观测到达时间，超时/缺失仅作为有依据的推断；FCS 错误只报告捕获路径实际暴露的值。增加由被动观测构建的拓扑、质量基线阈值、诊断报告和 MQTT 告警，不主动扫描或注入 OT 流量。协议头解析参考 Wireshark dissector 字段与异常语义并采用独立 C 实现；不链接其 GPL 代码，也不解密加密载荷。

## Technical Context

**Language/Version**: C11；Linux userspace runtime。平台和驱动差异由 HAL 实现吸收，分析核心不依赖特定交换机 SDK。

**Primary Dependencies**: 稳定的 HAL 收报接口、现有管理界面、设备时间服务、安全凭据管理能力，以及支持 TLS 和客户端身份认证的 MQTT 客户端/代理配置。协议分析模块只依赖平台无关 HAL 契约，不感知 ASIC、CPU、操作系统或 SDK 差异；硬件差异由 HAL 实现屏蔽。

**Storage**: 默认保留最近 7 天的聚合统计和事件；用户可以设置更长的保存时长。按设备存储能力管理可用上限；不保存 OT 应用层原始载荷。

**Testing**: 使用离线协议样本/回放与目标设备流量资格验证；Linux 原型可使用本机 C 工具链构建，设备侧 HAL 资格验证由平台集成方执行。

**Target Platform**: 具备面板端口报文复制能力且实现本功能 HAL 收报契约的交换机。分析模块不绑定特定型号、ASIC、CPU、操作系统或 SDK；平台适配由独立 HAL 实现提供。

**Project Type**: 可移植 C 分析模块，在 Linux userspace 运行；复用宿主产品现有管理 UI，不单独引入服务器或移动端。

**Performance Goals**: 在选定设备的声明支持镜像负载内，解析器持续处理输入且观测丢弃可见；资源占用不影响交换转发和控制面基础服务；统计更新满足设备管理 UI 的既有刷新体验。以目标 SKU 的实测资格矩阵确定并发布具体 pps、带宽、时间戳精度、更新间隔和 CPU/内存上限。

**Constraints**: CPU 复制流量为尽力观测输入；ASIC/驱动可能过滤坏 FCS 帧、截断或丢弃过量镜像流量。分析器必须单独计量采集丢弃、解析失败和业务推断，不能将其混为 OT 网络丢包。不能修改或阻断被监测的 OT 流量。

**Scale/Scope**: EtherNet/IP（显式与可识别的隐式通信）、PROFINET（NRT、RT/IRT 相关帧）、Modbus TCP、OPC UA TCP 和 MQTT 的可见协议字段；含有界 TCP 序号分析和 PDU 流重组，不含 TLS/OPC UA 内容解密及任意原始载荷留存。端口数、会话/关系数、重组窗口、更长保存窗口的上限与速率由目标 SKU 定义，默认保存窗口为 7 天。

## Constitution Check

仓库的 `.specify/memory/constitution.md` 仍为未填写的模板，没有生效的项目原则、技术限制或质量门禁。因此本计划没有可检查的宪章条款，也没有需要豁免的违例。实施时仍需遵循目标产品既有安全、资源与发布要求。

**Gate (before research)**: PASS — 没有已声明的强制原则阻挡研究或方案设计。

## Project Structure

### Documentation (this feature)

```text
specs/001-industrial-protocol-analysis/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
└── contracts/
    └── ot-observability.md
```

### Source Code (planned logical modules; map into target product repository)

```text
switch-agent/
├── platform/       # ACL mirror input, device counters, time source, resource guard
├── capture/        # frame normalization, capture health and drop accounting
├── protocols/      # Five OT protocol recognizers and bounded frame decoding
├── analysis/       # flow correlation, quality metrics, assets/topology, baselines, events
├── store/          # bounded aggregate/event retention; no raw payload by default
├── reporting/      # acceptance and diagnostic summaries
├── notifications/  # TLS-authenticated MQTT event delivery, credential references, and status
└── presentation/  # existing switch management view integration

qualification/
├── traces/         # synthetic/de-identified reference traces
└── profiles/       # per-SKU workload and expected capacity limits
```

**Structure Decision**: The repository initially contained only Spec Kit metadata and feature documents. The C11 module is organized into portable logical layers; all vendor-specific capture/time/resource integration stays behind a stable HAL boundary. Protocol decoding, metric semantics, storage, reporting, and presentation remain platform-neutral. The analyzer receives packet records through HAL and does not invoke vendor SDK APIs directly.

**MQTT Security Decision**: MQTT alarm delivery uses TLS transport and broker authentication. The notification module must use the switch's secure credential-management facility and must not expose credential values in the management UI or logs. TLS/authentication setup and failures are surfaced as target connection state; delivery failure does not remove local events.

## Complexity Tracking

No constitution violations or architectural exceptions require justification.

## Phase 0: Research Gate

See [research.md](research.md). The protocol distinctions, passive-observation limits, and product reference findings are defined. Device-specific throughput and resource ceilings are intentionally defined by a target-SKU qualification profile before implementation, because no universal value can be inferred without the switch model.

## Phase 1: Design Gate

See [data-model.md](data-model.md), [contracts/ot-observability.md](contracts/ot-observability.md), and [quickstart.md](quickstart.md). The post-design gate passes: protocol/metric scope is bounded, topology and external notifications use passive evidence, data confidence is carried through to the UI, and no solution depends on visibility the mirrored capture path may not provide.

**Gate (after design)**: PASS — target platform qualification remains an explicit implementation prerequisite, not an unresolved user-visible metric definition.
