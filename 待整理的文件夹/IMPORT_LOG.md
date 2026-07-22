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
