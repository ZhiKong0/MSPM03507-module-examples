# 待整理的文件夹

这里是 MSPM0G3507 / Cortex-M0+ 电赛例程候选池。当前不要把这些代码视为“已可直接比赛使用”的正式模块；它们先作为有来源、有许可证状态、有适配价值判断的原始材料保存。

## 先看哪里

1. `00_整理工作台/GOAL.md`：本轮统一整理 goal、边界和验收标准。
2. `00_整理工作台/SOURCE_INVENTORY.md`：当前候选池的来源、规模和风险盘点。
3. `00_整理工作台/MODULE_TAXONOMY.md`：后续正式拆模块时采用的统一分类。
4. `00_整理工作台/FIRST_PASS_ACTIONS.md`：第一批应该优先抽查、清理和迁移的文件路线。
5. `CANDIDATES.md`：完整候选清单。
6. `IMPORT_LOG.md`：导入过程日志。
7. `SOURCE_POLICY.md`：来源与许可证处理规则。

## 当前目录含义

- `official_ti_sdk/`：TI 官方 SDK 候选，作为外设例程和寄存器/DriverLib 参考基准。
- `github/`：GitHub 来源候选，主要是电赛小车、模块驱动、工程模板、视觉/调参方案。
- `gitee/`：Gitee 来源候选。
- `gitcode/`：GitCode 来源候选。

## 整理原则

- 先建索引，再拆代码。
- 先官方基准，再第三方增强。
- 先 CMake/Makefile + ARM GCC 路线，再兼容 Keil/CCS/SysConfig。
- 无许可证或许可证不适合的代码只做学习参考，不直接并入正式模块。
- 有实机验证记录的第三方工程优先抽查，但必须重新做本地构建和硬件验证。
