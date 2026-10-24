# Quickstart: 工业协议通信分析资格验证

本指南为实现阶段准备端到端验证流程。当前仓库没有目标交换机镜像、协议工具链或产品 UI，因此命令需由目标 SDK/测试环境提供，不能在此仓库直接运行。

## Prerequisites

- 已选定交换机 SKU、ASIC、CPU、OS/SDK 版本和 ACL 到 CPU 的复制模式。
- 准备隔离的验证网络、已知 EtherNet/IP、PROFINET、Modbus TCP、OPC UA TCP 和 MQTT 流量源，以及不影响生产控制的测试窗口。
- 准备可重复的参考流量：EtherNet/IP 显式请求/响应、可识别的隐式 I/O；PROFINET 发现/诊断和周期流量；Modbus TCP 读写/异常响应；OPC UA HEL/ACK/安全通道帧；MQTT 控制与发布帧；再准备缺失响应、可见序列缺口、未知/截断报文和带外的硬件端口错误计数样例。
- 准备目标平台的基线转发/控制面负载测量办法和 ACL/CPU 复制计数器。
- 参考 [数据模型](data-model.md)、[UI 与采集契约](contracts/ot-observability.md) 及 [研究决定](research.md)。

## Validation flow

1. 在隔离环境中选择要监测的面板端口并启用分析；确认 UI 显示监测范围及可用状态。
2. 回放两种协议的已知样本，逐个比对端点、通信关系、业务类别、报文数/字节数与参考摘要。
3. 对能关联的往返事务核对 RTT；对周期流量核对到达间隔偏差；确认单向或缺少关联字段的样本不显示可用 RTT。
4. 注入预期响应缺失和可见序列缺口，确认系统分别显示 timeout/gap 证据与推断可信度，不直接断言网络丢包。
5. 检查校验信息：仅当 SDK 提供逐帧校验元数据时，核对逐帧错误；否则确认 UI 标记不可用，并单独展示可得的端口错误计数。
6. 施加超过目标资格配置的镜像输入负载，确认捕获/解析退化状态可见，捕获丢弃与协议推断分开统计，转发与控制面仍在目标基线范围内。
7. 检查各视图使用同一时间/协议/端点筛选条件，未知、加密、私有和截断流量保留在未知类别，不计入已识别业务。
8. 重启设备或重置计数器，确认统计代次切分，时间基准变化不会跨区间计算时延和速率。
9. 确认默认保存时长为 7 天；设置更长时长后，检查 UI 显示新值，并确认新配置保留期内的汇总数据可查询。
10. 使用新设备首次出现和控制器访问变化的样本，确认被动拓扑和资产信息更新，并能在图/表中看到身份置信度及端口数据来源。
11. 检查具有抖动样本的每个设备详情是否展示关联关系的抖动及样本范围；对周期稳定的 PROFINET 流量核对刷新周期/刷新率；无样本时显示不可用原因。
12. 建立完整和不完整的基线窗口；完整窗口应提供可审核阈值建议，不完整或时间未同步的窗口不得自动启用阈值。
13. 生成诊断/验收报告并配置 MQTT 告警目标；核对报告字段、告警证据及发布成功/失败状态，确认发送失败后本地事件仍保留。

TCP 协议 PDU 可在每方向 2 KiB 乱序窗口和 8 KiB PDU 缓冲上限内跨段重组；超过上限或缺片时显示不完整/重组溢出。配置采集覆盖状态后核对 TCP 流诊断：下一期望序号、缺口候选、重传/乱序、重复 ACK、ACK RTT 和超时候选。读取 TCP 汇总 API 的活动方向流数、容量拒绝数和 RTO；存在容量拒绝时必须把流清单标为不完整。镜像不完整时缺口必须保持“未捕获/原因不确定”，不得显示成确认网络丢包。TLS 和 OPC UA 加密内容不解密，协议载荷不进入持久化记录或诊断快照。

## Expected outcome

- 已识别协议业务类别和流量计数与参考样本一致。
- RTT、jitter、timeout、gap 和 checksum 指标只在其观测前提满足时给出；其余情况显示原因或不确定性。
- 超过设备资格负载后，界面明确说明观测不完整；不将 analyzer capture drop 误报为 OT network drop。
- 完成每 SKU 资格记录：持续支持 pps/bytes/s、监测端口/关系数、捕获丢弃率、时间戳精度、UI 更新间隔、CPU/内存使用和转发影响。数值由目标硬件验证确定后写入相应设备 profile。
# C/Linux Module Integration

## Interface boundaries

The analyzer has two boundaries:

1. **Input from the platform**: implement `ot_hal_capture_t` from `switch-agent/platform/hal_capture.h`. The module calls `receive(context, timeout_ms, packet)` and optionally `get_counters`. Implementations translate ACL mirror/driver APIs into this record. The analyzer has no vendor SDK dependency.
2. **Output to the host product**: include `switch-agent/module_api.h`; create the opaque instance with `ot_module_create`, set known mirror coverage and TCP RTO, feed frames using `ot_module_ingest` or poll an attached HAL with `ot_module_poll`, call `ot_module_tick` with the same monotonic clock for timeout updates, and read typed overview/TCP snapshots. Call `ot_module_destroy` during shutdown. The host owns UI rendering, account authorization, persistence, and delivery services.

`metadata_flags` determines whether ingress port, direction, timestamp, checksum status, and counters are available. Missing flags mean unavailable; the analyzer must not synthesize values. The HAL owns frame memory and must keep it valid until the next `receive` call. The bounded packet queue copies frame bytes when the host submits them for queued processing.

## Build

The module is C11 and builds as a static library with CMake or a compatible C compiler. On MSYS2 MinGW64, ensure `C:\msys64\mingw64\bin` and `C:\msys64\usr\bin` are on `PATH` so GCC can load its compiler runtime DLLs.

```sh
cmake -S switch-agent -B switch-agent/build
cmake --build switch-agent/build
```

## Capability qualification

The HAL implementation should report only supported metadata and counters. Product integration must document packet rate, queue/drop counters, timestamp resolution, counter resets, available error/FCS data, and resource state for each host environment. Unsupported capabilities remain unavailable in module output. No SKU-specific conditional logic belongs in `switch-agent/protocols/` or `switch-agent/analysis/`.
