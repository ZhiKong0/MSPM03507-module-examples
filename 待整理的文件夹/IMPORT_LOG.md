# 导入日志

## 2026-07-22

### 已完成

- 建立本地仓库：`E:\Learning\嵌入式\电赛备赛\MSPM03507各模块例程代码`
- 建立候选目录：`待整理的文件夹/`
- 建立 GitHub submodule 候选入口：21 个
- 建立候选清单和来源策略文档

### 未完成

- 远程 GitHub 仓库尚未创建：`gh` 当前 token 缺少创建仓库所需 scope，`gh auth refresh` 的 OAuth token 请求超时。
- `Ykdzds/TI2026_jy61p_car_temp_proj` 当时尚未成功导入：两次 clone 都因 GitHub 连接超时失败；此项已在第二轮持续检索中重试成功。

### 下一轮目标

1. 修复 GitHub CLI 授权后创建远程私有仓库并 push。
2. 重试 `Ykdzds/TI2026_jy61p_car_temp_proj`。
3. 继续按模块检索：电机驱动、编码器、灰度阵列、IMU、OLED 菜单、蓝牙/串口调参、K230/MaixCam/OpenMV 接口。
4. 对 A 级候选做结构扫描，整理出可直接拆分的模块列表。

## 2026-07-22 第二轮持续检索

### 已完成

- 继续用 GitHub 搜索检索 MSPM0G3507 / M0 电赛候选。
- 将 `Ykdzds/TI2026_jy61p_car_temp_proj` 重试导入成功。
- 新增第三批 GitHub submodule：16 个。
- 当前 `.gitmodules` 有效候选入口：37 个。

### 本轮新增重点

- 2024 H/电赛自动驾驶小车：`bigandstone/TI-Cup-2024-ProblemH-AutoCar`
- 八路灰度 + TB6612 + 编码器 PI/巡线 PD：`5ee511/MSPM0G3507-Car`
- 天猛星智能车模板：`persica123/TMX_MSPM0G3507_ProjectTemplate`
- JY61P/Track Motor 模块：`glf-loser/mspm0g3507-modules`
- 实际电赛项目驱动库：`dzzz-qcxf-studio/MSPM0_Driver_Lib`
- ER2024 巡线+作业平台：`zhujs1103/er2024`

### 未完成

- 远程 GitHub 仓库仍未创建：本机 `gh` token 缺少创建仓库所需 scope。
- `WYJxxoo/MSPM0G3507_syscfg_freeRTOS` 与 `xieyangyingshutong/25xiaoche` clone 中断，已作为待重试候选记录。
- 仍有若干检索到但未导入的候选：OpenMV 视觉追踪、校级赛题合集、2026 Vehicle、跟踪目标小车、Keil 闭环工程等。

## 2026-07-22 远程仓库创建

### 已完成

- 已通过登录态 Chrome 网页创建 GitHub 私有仓库：`https://github.com/ZhiKong0/MSPM03507-module-examples`
- 已将本地仓库设置远程：
  - `origin = https://github.com/ZhiKong0/MSPM03507-module-examples.git`
- 已推送 `main` 分支到远程，并设置本地 `main` 跟踪 `origin/main`。

### 命名说明

- 用户指定的中文仓库显示名为：`MSPM03507各模块例程代码`。
- GitHub 新仓库页面实测限制仓库名只能包含 ASCII 字母、数字、`.`、`-`、`_`。
- 因此远程仓库 slug 使用兼容名称：`MSPM03507-module-examples`；中文正式名称继续保留在 README、描述和候选文档中。

## 2026-07-22 第三轮补导入

### 已完成

- 补导入上轮待导入候选：6 个。
- 当前 `.gitmodules` 有效候选入口：43 个。
- 新增候选包括：
  - `xy1092/nuedc_control_car_freertos_template`
  - `bigandstone/TI-Cup-School-Competitions`
  - `military-doge/Grayscale-Line-Follower-Robot`
  - `chenjingquan-hntb/Nuedc2026-MSPM0G3507-Vehicle`
  - `asdfguai/mspm0g3507-tracking-target-car`
  - `likechen0/four-drive-yabo-sensor`

### 未完成

- `Students800/Find_box_car` 是空仓库，不能形成有效 submodule；保留为 source-card。
- `iwiwb/MSPM0-keil-project` 和 `QwQuintOO/MSPM0G3507_Template` 本轮因 GitHub 连接重置未导入，已保留为待重试。
- `WYJxxoo/MSPM0G3507_syscfg_freeRTOS` 和 `xieyangyingshutong/25xiaoche` 仍保留为待重试。

## 2026-07-22 第四轮持续检索

### 已完成

- 重试导入成功：
  - `iwiwb/MSPM0-keil-project`
  - `QwQuintOO/MSPM0G3507_Template`
- 新增第四批候选：
  - `xujh666/mspm0g3507-line-tracking-car_v1.0`
  - `xujh666/mspm0g3507-line-tracking-car`
  - `chowhound-eat/my-car-electronic-contest-prep`
  - `Yang-runze/mspm0g3507-smart-car`
  - `summer-xiying/MOTOR_PID_25H`
  - `dawn-qvq/25-e-`
  - `aluomx/Nuedc-code`
  - `Eliauk-Camelia/mspm0g3507-imu-lcd`
- 当前 `.gitmodules` 有效候选入口：53 个。

### 未完成

- `WYJxxoo/MSPM0G3507_syscfg_freeRTOS` 与 `xieyangyingshutong/25xiaoche` 本轮仍因 GitHub 连接失败未导入。
- `kokona23/mspm0G3507cartest_record` 本轮 GitHub 连接重置。
- `sbjsw334/TI_MSPM0G3507_Project` clone 超时，已结束残留 clone 进程并清理半成目录。
- `Tacrine/TMC2209_StepMotor_Driver` 尚未导入，作为步进电机专项候选保留。

## 2026-07-22 第五轮持续检索

### 已完成

- 继续通过 GitHub 检索 `MSPM0G3507`、小车、巡线、PID、FreeRTOS、VSCode/GCC 工程模板相关候选。
- 新增第五批候选并导入 5 个 GitHub submodule：
  - `ice-cream-1024/-MSPM0G3507_car_control`
  - `xiaoshuai361/mspm0g3507_template`
  - `Huai-jin-6/m0template`
  - `Almighty42/MSPM0G3507-System-Library`
  - `ergouzi332/26_TI_MSPM0G3507`
- 当前 `.gitmodules` 有效候选入口：58 个。

### 未完成

- `686my686/MSPM0G3507-AutoCar`：clone 超时，已结束残留 `git-remote-https` 并清理半成目录。
- `RitsuYi/MSPM0G3507_VSCODE`：GitHub 连接重置，保留待重试。
- `Ju4tdo1/MSPM0G3507_FreeRTOS`：GitHub 连接重置，保留待重试。
- `jeffrey070318/MSPM0G3507_BASIC-`：仓库内容高度相关，但本轮导入后在 Windows 子模块工作树中出现 `git-lfs filter-process` 残留和索引异常；已从索引撤回并清理工作目录，改为待重试候选。

## 2026-07-22 第六轮 code search 检索

### 已完成

- 仓库名关键词检索暂未发现新的未导入高相关仓库。
- 改用 GitHub code search 反查模块关键词，命中 2 个仓库名不直接暴露 MSPM0G3507 小车特征的候选：
  - `VincentttWang/NUEDC_2025`：命中 `2025E/工程文件/BSP/Motor_Encoder/TB6612FNG_README.md`
  - `lxl2024666/EDC_ti`：命中 `Template/BSP/Motor_Encoder/TB6612FNG_README.md`
- 两者已登记为第六批 source-card/待重试候选。

### 未完成

- `VincentttWang/NUEDC_2025`：`git ls-remote`/clone 过程中 GitHub 连接重置，暂不导入 submodule。
- `lxl2024666/EDC_ti`：普通 clone 与 `--depth 1` 浅克隆均连接重置，已确认未留下半成目录。

## 2026-07-22 第七轮持续检索与补导入

### 已完成

- 宽搜 `MSPM0 car`、`MSPM0 smart car`、`MSPM0 line follower`、`MSPM0 2025E`、`天猛星 MSPM0G3507` 等关键词，新增高相关候选：
  - `DQ103/mspm0-car-2024-h`
- `DQ103/mspm0-car-2024-h` 已确认非空、MIT 许可；含 CMake、FreeRTOS、SysConfig、Control、Drivers、tools，以及多个小车调试分支。
- 从待重试池补导入成功 5 个 GitHub submodule：
  - `686my686/MSPM0G3507-AutoCar`
  - `Ju4tdo1/MSPM0G3507_FreeRTOS`
  - `VincentttWang/NUEDC_2025`
  - `xieyangyingshutong/25xiaoche`
  - `Tacrine/TMC2209_StepMotor_Driver`
- 本轮总新增 submodule：6 个。
- 当前 `.gitmodules` 有效候选入口：64 个。

### 未完成

- `lxl2024666/EDC_ti`：本轮 `git ls-remote` 一度可达，但 submodule 普通 clone 仍失败，错误为无法连接 GitHub 443 端口；继续保留 source-card。
- `RitsuYi/MSPM0G3507_VSCODE` 与 `WYJxxoo/MSPM0G3507_syscfg_freeRTOS`：本轮 `ls-remote` 仍不可达。
- `kokona23/mspm0G3507cartest_record`：本轮 clone 连接重置，继续保留待重试。
- `sbjsw334/TI_MSPM0G3507_Project`：本轮 clone 连接 GitHub 443 失败，继续保留待重试。

## 2026-07-22 第八轮全网/Gitee/GitCode 检索

### 已完成

- 使用 Agent Reach 的 Exa 全网语义搜索，关键词覆盖 `MSPM0G3507 电赛 小车 例程 代码 Gitee`、`MSPM0G3507 循迹 小车 代码`。
- 新增 5 个可 Git 拉取的 submodule：
  - `https://gitee.com/szturin/mspm0-g3507_-robo-car.git`
  - `https://gitee.com/laoguaige/electric-racing-car.git`
  - `https://gitee.com/namelesstech/lp_mspm0g3507_mini_examples.git`
  - `https://github.com/2262727886-stack/mspm0g-contest-skill.git`
  - `https://gitcode.com/open-source-toolkit/b12ea.git`
- 新增 1 个页面/附件型 source-card：
  - `https://oshwhub.com/relinking/automatic-driving-car-based-on-g`
- 当前 `.gitmodules` 有效候选入口：69 个。

### 许可证/使用提示

- `laoguaige/electric-racing-car`：License 文件为 CC BY-NC-SA 4.0，仅作非商业学习参考。
- `namelesstech/lp_mspm0g3507_mini_examples`：License 文件为 GPL-3.0，后续不得直接并入非 GPL 主线。
- `open-source-toolkit/b12ea`：License 文件为 MIT，但其仓库内 zip 的内容仍需后续解压核验。
- `szturin/mspm0-g3507_-robo-car`：License 文件为空，按未声明处理。
- `2262727886-stack/mspm0g-contest-skill`：未发现顶层 License 文件，按未声明处理。

### 未完成

- `JamieK32/ti-contest`：`ls-remote` 可达，仓库页面可读，但本轮 submodule clone 连接重置；已登记为待重试。
- 立创开源硬件平台 `automatic-driving-car-based-on-g`：页面提示有附件 `Car_v4.1.zip`，但属于页面附件型来源，本轮不直接下载或复制，先登记 source-card。
