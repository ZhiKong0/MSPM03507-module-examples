# 10 工具链模板

状态：`source-index`

## 模块目的

统一 MSPM0G3507 电赛例程的本地构建、产物生成和烧录入口，优先服务 `CMake/Makefile + arm-none-eabi + pyOCD/DAPLink`，并记录 probe-rs、EIDE、CCS、Keil 的迁移价值。

## 首批候选来源

| 候选 | 用途 | 当前判断 |
|---|---|---|
| `待整理的文件夹/github/Cxxhh__MSPM0G3507-CMAKE-GCC-Template` | CMake + ARM GCC 模板、hex/bin 产物、probe-rs 下载 | `build-candidate` |
| `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/*/gcc/makefile` | 官方 Makefile/SysConfig 命令参考 | `source-index` |
| `待整理的文件夹/github/Ctrl-CVCV__TI-EIDE-Example` | EIDE/VS Code 工程经验 | `porting-candidate` |

## 证据路径

- `待整理的文件夹/github/Cxxhh__MSPM0G3507-CMAKE-GCC-Template/README.md:3`：README 标明 CMake、ARM GCC、Ninja、Probe-rs 工作流。
- `待整理的文件夹/github/Cxxhh__MSPM0G3507-CMAKE-GCC-Template/README.md:50`：记录构建命令入口。
- `待整理的文件夹/github/Cxxhh__MSPM0G3507-CMAKE-GCC-Template/CMakeLists.txt:36` 到 `:49`：声明 Generic ARM 系统与 `arm-none-eabi-` 工具链。
- `待整理的文件夹/github/Cxxhh__MSPM0G3507-CMAKE-GCC-Template/CMakeLists.txt:67` 到 `:69`：Cortex-M0+ 编译参数。
- `待整理的文件夹/github/Cxxhh__MSPM0G3507-CMAKE-GCC-Template/CMakeLists.txt:84` 和 `:88`：生成 hex/bin。
- `待整理的文件夹/github/Cxxhh__MSPM0G3507-CMAKE-GCC-Template/run.sh:67` 到 `:70`：probe-rs 下载和复位流程。
- `待整理的文件夹/official_ti_sdk/mspm0_sdk_2_11_00_07/examples/nortos/LP_MSPM0G3507/driverlib/uart_rw_multibyte_fifo_poll/gcc/makefile:11` 到 `:15`：官方 Makefile 调 SysConfig 生成文件。

## 下一步

1. 形成一个统一 `toolchain_contract.md`：输入源码、链接脚本、SysConfig 输出，产物为 elf/hex/bin/map。
2. 增加 pyOCD/DAPLink 验收命令，不把 probe-rs 作为唯一烧录路径。
3. 给每个正式模块补 `build-notes.md`，明确“可构建”和“仅迁移参考”的边界。

## 许可和构建备注

模板来源需逐项检查许可证。正式模块中若直接复制模板脚本，要保留原许可证；若仅吸收结构，应重写为本仓库统一构建脚本。
