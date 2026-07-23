# 巡线模块 Goal/Loop 规格 v0.1

## 1. Goal

把现有巡线模板打磨成可验证、可复用、可通过 SWD/运行时热调参的稳定模块。最终模块必须能移植到 MSPM0G3507 小车工程，支持 PC/mock 验证、MSPM0 构建、台架传感器验证、低速车测和热调参车测。

## 2. Non-goals

- 首轮不做摄像头巡线。
- 首轮不做完整整车赛题应用。
- 首轮不做 GUI 调参上位机。
- 首轮不实现完整 Flash A/B 参数区，只保留 RAM 调参和未来持久化边界。
- 不复制无许可证或许可证不清晰的第三方源码。

## 3. Stable Requirement IDs

| ID | Requirement | Status |
|---|---|---|
| G-001 | 当前主线只打磨现有巡线模板 | confirmed |
| G-002 | 复用边界为纯算法库 + MSPM0G3507 适配 + 调参/验证示例 | confirmed |
| G-003 | 覆盖 GPIO、模拟阈值数字化、I2C；排除摄像头 | confirmed |
| G-004 | 主接口升级为统一采样帧，逐通道读取保留兼容 | confirmed |
| G-005 | 调参参数分算法控制和传感器适配两类 | confirmed |
| G-006 | SWD 调参采用 RAM 参数块 + 显式 apply + MCU 校验 | confirmed |
| G-007 | 运动中热调参必须受 RUNNING_TUNE_SAFE、限幅、回读、回滚约束 | confirmed |
| G-008 | 自反馈 loop 使用五层证据门 | confirmed |
| G-009 | v0.1 有量化稳定验收标准 | confirmed |
| G-010 | 输出统一 telemetry/evidence frame | confirmed |
| G-011 | 热调参数默认留 RAM，Flash commit 后续显式处理 | confirmed |
| G-012 | 首轮实施统一采样帧、调参块、telemetry、PC/mock、MSPM0 示例 | confirmed |

## 4. Roles and Triggers

- 开发者：移植模板、连接传感器、电机和调参入口。
- 调参者：通过 SWD/pyOCD 或运行时入口写 RAM 参数块，并读取 `status/error_code/telemetry`。
- 验证者：按五层证据门收集测试结果，不用肉眼感觉替代证据。
- 触发条件：新增巡线车项目、替换巡线传感器、换底盘、需要热调 PID/速度参数、低速车测不稳定。

## 5. Workflow

1. 先用 PC/mock 测试证明算法输入输出正确。
2. 再接入 MSPM0 工程并完成 GNU/CMake 或 Makefile 构建。
3. 台架读取真实传感器，确认 `raw_bits/active_bits` 和左右误差方向。
4. 低速跑车，先追求 3 圈或 3 分钟稳定。
5. 开启 `RUNNING_TUNE_SAFE`，通过 SWD 小步修改热参数。
6. 读取 telemetry 和 tuning 状态，判断传感器、算法、速度环或调参 apply 的问题来源。
7. 将可复用参数回写到配置或后续 Flash commit 流程。

## 6. Constraints

- 算法层不直接读 GPIO、ADC、I2C，也不直接操作电机。
- GPIO、模拟阈值和 I2C 都必须先转换成 `line_trace_sample_frame_t`。
- 普通 `RUNNING` 不接受热写；运动中调参必须进入 `RUNNING_TUNE_SAFE`。
- 越界参数、危险突变、冷参数运动中变化必须被拒绝或回滚。
- `LineTrace_Update()` 只作为旧 GPIO 逐通道兼容入口。
- 默认不写 Flash，防止把偶然可跑或危险参数固化。

## 7. Verification

五层证据门见 `docs/VERIFICATION_MATRIX.md`。v0.1 首轮必须至少完成：

- PC/mock 测试通过。
- host 编译无警告级错误。
- 代码结构中能看到统一采样帧、调参块、telemetry 和 MSPM0 示例。

硬件相关证据可以在后续 loop 补齐，但不能用 host 测试冒充台架和车测完成。

## 8. Change Log

- v0.1：确认需求边界 G-001..G-012。
- v0.1 implementation slice：新增统一采样帧、RAM 调参块、telemetry、PC/mock 测试和 MSPM0 示例。
- deferred：Flash A/B commit、GUI 调参、摄像头、完整整车应用。
