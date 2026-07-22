# 统一整理 Goal

## Goal

把当前 MSPM0G3507 例程候选池从“收集仓库和官方 SDK 的资料湖”，整理成可追踪、可筛选、可迁移、可验证的电赛模块库准备区。

## 具体目标

1. 建立统一分类：官方基准、工具链模板、BSP/外设、底盘闭环、巡线传感、IMU 姿态、显示菜单、串口/蓝牙调参、视觉协处理器、完整赛题应用。
2. 给每个候选来源标出角色：`official-baseline`、`toolchain-template`、`module-reference`、`competition-app`、`reference-only`。
3. 给每个后续要拆的模块标出状态：`source-only`、`build-candidate`、`porting-candidate`、`runtime-proven`、`license-restricted`。
4. 优先服务 2026 控制题备赛主线：差速底盘、编码器、灰度/光电、IMU、OLED/按键菜单、串口/蓝牙调参、K230/MaixCam/OpenMV 视觉接口。
5. 最终整理出的正式模块必须能走本地偏好的 `CMake/Makefile + ARM GCC + pyOCD/DAPLink` 验证链；Keil/CCS/SysConfig 工程只作为兼容参考。

## 非目标

- 不把所有候选源码一次性混合成一个正式工程。
- 不直接修改上游 submodule 的内部历史。
- 不把无许可证、GPL/CC BY-NC-SA 等限制性来源的代码直接并入非兼容主线。
- 不把官方 SDK 的完整压缩包、预编译库、构建产物或 IDE 缓存当作正式源码提交。

## 验收标准

- 候选池有一个稳定入口：`待整理的文件夹/README.md`。
- 每轮整理都能回答三个问题：这个来源是什么、能拆什么模块、下一步如何验证。
- 第一批 A 级候选有明确阅读顺序和抽查路径。
- 后续新增来源必须能放入同一套分类和状态体系。
- 每次形成正式模块前，都能从本工作台追溯到原始来源、许可证和验证状态。

## 核心判断

当前仓库的正确整理方式不是“按来源堆文件”，而是建立两层结构：

- 来源层：保留官方 SDK、GitHub/Gitee/GitCode submodule 和页面型 source-card 的原始追溯。
- 模块层：按电赛真实功能拆出可复用模块，且每个模块必须标出来源、许可证、构建方式和验证等级。

这个 goal 继续沿用仓库根 README 已经写明的候选池定位：当前阶段不直接判断代码质量已可用于比赛，而是先保留来源、许可证、适配价值和整理状态，后续再拆模块。证据见 `README.md:5`、`README.md:17`、`README.md:18`。
