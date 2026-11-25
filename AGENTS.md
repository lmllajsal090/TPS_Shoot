# AGENTS.md — TPS_Shoot (UE 5.8 多人联机搜打撤 TPS)

> **定位**：本文件是 `TPS_Shoot` 项目的 **跨 AI 通用协作入口（Single Source of Truth）**，适用于本地所有具备仓库读写能力的 Coding Agent（Antigravity、ChatGPT/Codex 等）。
> **项目优先级**：本项目是用户面向 **3 月实习求职最主打的旗舰 Demo**，技术核心聚焦于 **多人联机同步与优化** 与 **高扩展性敌人战斗框架**。

---

## 1. 项目概览与核心目标

- **项目路径**：`C:\UEProjects\TPS_Shoot_Original_5_8`（引擎版本：**Unreal Engine 5.8**）
- **远端仓库**：`https://github.com/lmllajsal090/TPS_Shoot.git`
- **架构风格**：**纯 UE C++ 组件化架构**（不使用 GAS，与另一项目 `Aura_5_8` 形成技术互补）。
- **核心玩法闭环**：可联机 TPS 类《三角洲行动》**搜-打-撤**模式（探索交火 -> 夺取第一关高价值目标 **「非洲之星」** -> 前往撤离点倒计时守点 -> 撤离结算）。
- **核心技术亮点排序**：
  1. **第一亮点：联机同步与网络优化**（表现先行消除延迟、精准复制条件、高频弹道同步优化、弱网压测与带宽量化、联机会话入口）。
  2. **第二亮点：高扩展性敌人战斗框架 + 远程敌人**（在现有 `EnemyBase + EnemyCombatComponent + UEnemyCombatConfig` 上解耦近战/远程攻击策略，接入 EQS 战术站位）。
  3. **第三亮点：武器闭环（冲锋枪 + 手枪 + 近战小刀）与精简实用 UI / 3C 动作**。

---

## 2. 文档索引（按需阅读 · 渐进式披露）

| 文档 | 内容 | 何时阅读 |
| :--- | :--- | :--- |
| `README.md` | 面向人类与面试官的项目门面、系统架构图与优化成果展示 | 阶段收口同步更新 |
| `Docs/01_Architecture.md` | 当前 C++ 类地图、网络复制拓扑、武器/敌人状态机与资产索引 | 新会话首次接手或查代码结构时 |
| `Docs/02_Roadmap.md` | 阶段里程碑总看板与优先级 | 规划任务或核对功能范围时 |
| `Docs/03_Progress_Log.md` | 开发进度、架构决策记录（ADR）与下一步交接点 | **每次新会话必读** |
| `Docs/Specs/*.md` | 单系统 Spec + TDD 设计书（含 `_TEMPLATE.md` 模板） | **开发任意新系统前必读对应 Spec，先对齐定稿再写代码** |

---

## 3. AI 协作模式（严格遵守）

### 默认模式：Mode A（副驾驶 / 导师模式）
除非用户明确要求切换，**所有 AI 默认处于 Mode A**：
1. **先对齐 Spec/设计再谈代码**：收到需求后，先分析架构扩展性、联机同步方案（权威端/RPC/复制条件/带宽成本）与蓝图边界，给出 1~2 种方案对比。
2. **给接口骨架与思路，不越权全包**：确认方案后，优先提供头文件接口草案、核心状态机步骤与避坑点，由用户主导实现，AI 负责 **Code Review、编译/联机排错与面试深度追问**。
3. **禁止行为**：Mode A 下不得未经确认直接批量改写大量业务 `.cpp`。

### 按需切换：Mode B（主程 + 执行模式）
仅当用户明确指示（如“切换 B 模式”、“你来直接写并编译”）时进入 Mode B：
1. 动手前简明确认改动范围；实现后执行 UE 5.8 UBT 编译验证。
2. 完成后必须向用户讲清核心实现与联机设计意图，确保用户面试能完全掌控每一行代码。

---

## 4. 铁律：新增功能必同步更新 Markdown 文档（Living Documentation）

> **强制规则**：每当项目**新增一个功能、扩展一个子系统、或完成一项联机/架构优化**，在任务结束前**必须同步补充并更新对应的 Markdown 文档**，严禁“只改代码、不留文档”：
1. **对应功能 Spec（`Docs/Specs/*.md`）**：若为新功能，先按 `_TEMPLATE.md` 创建或补充其 Spec+TDD，并更新状态（`Draft` -> `Approved` -> `Implemented`）与实际落地细节。
2. **架构快照（`Docs/01_Architecture.md`）**：补充新增的类、组件、DataAsset、复制属性/RPC 与核心数据流。
3. **总路线图（`Docs/02_Roadmap.md`）**：勾选已完成项，或记录新增需求。
4. **进度与决策日志（`Docs/03_Progress_Log.md`）**：追加本次变更摘要、踩坑点、性能/联机实测数据及关键架构决策（ADR）。

---

## 5. 硬性工程红线（Networking & UE Safety Rules）

### 5.1 联机与网络优化红线（核心竞争力）
- **服务端权威（Server Authority）**：伤害结算、弹药扣减、切枪提交、敌人命中判定、目标物拾取/掉落、撤离倒计时与胜负判定**必须仅在服务端（`HasAuthority()`）执行**；客户端只发请求 RPC 与做本地表现。
- **拒绝无效带宽浪费**：
  - 仅归属玩家关心的状态（如自己弹药/备弹/个人任务提示）优先使用 `COND_OwnerOnly`；
  - 客户端已做本地表现先行的事件，同步给其他端时优先用 `COND_SkipOwner` 或在 Multicast 中跳过 `IsLocallyControlled()`，杜绝双播；
  - 坐标/法线等高频网络参数使用 `FVector_NetQuantize` / `FVector_NetQuantizeNormal`；
  - 谨慎使用 `NetMulticast, Reliable`（高频特效/弹孔/受击表现一律用 `Unreliable` 或属性状态同步，避免撑爆 Reliable Buffer 导致掉线）。
- **定时器与异步防串台**：所有延迟状态切换（切枪、换弹、敌人攻击前摇/后摇、撤离倒计时）必须在 `EndPlay` / 死亡 / 取消时清理 `FTimerHandle`，并优先参考 `UEnemyCombatComponent` 的递增 ID（`CurrentAttackId`）防旧回调重入。

### 5.2 蓝图与历史命名红线（切勿盲目改名）
- **禁止随意改名已暴露给蓝图的反射符号**（`UCLASS` / `USTRUCT` / `UPROPERTY` / `UFUNCTION`）。确需重构必须评估 `.uasset` 影响并在 `Config/DefaultEngine.ini` 配置 `[CoreRedirects]`。
- **当前已存在且绑定了蓝图资产的历史双前缀类名（请严格照用，切勿“顺手修正”）**：
  - `AAEnemyAIController`（`AIC_EnemyController` 父类）
  - `AAProjectileBase`（子弹蓝图父类）
  - `AATPSHUD`（`BP_TpsHUD` 父类）
  - `UUStateBarWidget`（`WBP_StateWidget` 父类）
  - `UUTPSMainHUD`（`WBP_MainHUD` 父类）
  - 文件名 `Source/TPS_Shoot/UI/AMmoWidget.h`（类名已是 `UAmmoWidget`，注意 `#include` 路径大小写）
- **禁止直接修改二进制 `.uasset` / `.umap` 文件**；编辑器配置改动需给出明确操作步骤。

### 5.3 C++ 与蓝图职责边界
- **C++ 负责**：组件状态机、武器/敌人数据资产结构、网络复制与 RPC、AI 感知与自定义 BT Task、撤离阶段机、伤害与部位倍率计算、HUD 委托绑定。
- **蓝图负责**：角色/敌人/武器资源装配、AnimBP 与 Montage、Niagara/音效/材质表现、UMG 排版样式、关卡布景与刷怪/撤离点摆放。

---

## 6. 构建与联机测试命令

- **命令行编译 Editor 目标（PowerShell）**：
  ```powershell
  & "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" TPS_ShootEditor Win64 Development -Project="C:\UEProjects\TPS_Shoot_Original_5_8\TPS_Shoot.uproject" -WaitMutex -FromMsBuild
  ```
- **弱网联机验证控制台命令（PIE 2+ Players，Listen Server 模式）**：
  - `Net PktLag=60` / `Net PktLagVariance=15` / `Net PktLoss=2`（模拟高延迟与丢包）
  - `stat net`（查看实时发送/接收带宽、包率与 Actor Channel 数量）
- **注意**：UE 编辑器打开时避免外部全量 UBT 覆写 DLL（请关闭编辑器编译或使用 Live Coding）。
