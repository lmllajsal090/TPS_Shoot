# TPS_Shoot 开发进度与架构决策日志（Progress & ADR Log）

> **跨 AI 交接棒**：每次在 Antigravity 或 GPT/Codex 中开启新对话，请先读取本文件底部的「当前状态与下一步」。
> **强约束**：每次新增功能、修改架构或完成网络优化，必须同步更新本日志与对应的 `Docs/Specs/*.md`、`Docs/01_Architecture.md`、`Docs/02_Roadmap.md`。

---

## 当前状态与下一步（Next Session Entry Point）

- **当前阶段**：Phase 1 完成，已敲定 3 月前旗舰 Demo 总规划与跨 AI 文档底座
- **工作区现状**：`upgrade/ue5.8` 分支，当前有未提交的 `BP_Whisper` / `BT_Enemy` / `DA_WhisperCombatConfig` 等敌人战斗配置改动
- **下一步建议**：
  1. 确认并提交当前工作区的敌人战斗蓝图与本次新增文档底座。
  2. 决定先启动 **Phase 2（搜打撤闭环：非洲之星 + 撤离点）** 还是先攻克 **Phase 3（联机开火先行 + 弹道/带宽优化专项）**，并在 **Mode A** 下逐项定稿对应 Spec 后开工。

---

## 变更日志（Changelog）

### 2026-11-25 — 旗舰项目定位确认与 GitHub 级跨 AI 文档初始化
- **完成内容**：
  1. 全面盘点 `TPS_Shoot_Original_5_8` 全部 C++ 源码与蓝图资产，确认现有双槽武器、ADS、复制血量、以及高完成度的敌人近战状态机基线。
  2. 明确核心玩法定位：**可联机 TPS 类《三角洲行动》搜-打-撤模式**（第一关目标「非洲之星」+ 撤离点倒计时守点结算）。
  3. 明确核心亮点优先级：**联机同步与网络优化（第一核心）** > **高扩展敌人框架与远程 AI（第二核心）** > **小刀 3 号槽与极简 UI/3C**。
  4. 建立完整文档体系：`README.md`、`AGENTS.md`（内置「新增功能必同步更新 Markdown」强约束）、`Docs/01_Architecture.md`、`Docs/02_Roadmap.md`、`Docs/03_Progress_Log.md`、`Docs/Specs/_TEMPLATE.md` 及 4 份核心 Spec 草案。
  5. 补齐仓库根目录 `.gitattributes`，将 `*.uasset` 与 `*.umap` 纳入 Git LFS 托管。
  6. 完成受击反馈系统架构设计定稿（`Docs/Specs/05_Hit_Feedback_System.md`）：选定 `UHealthComponent` 统一收口伤害上下文 + `UHitFeedbackComponent` + `UHitFeedbackConfig`（DataAsset）解耦方案（ADR-005）。

---

## 架构决策记录（ADR）

### ADR-001：核心玩法定型为类《三角洲行动》多人搜打撤闭环
- **背景**：项目已有射击手感与近战敌人，但缺乏完整的局内目标、驱动力与结算闭环。
- **决策**：采用「夺取高价值目标（非洲之星）-> 撤离点守点倒计时 -> 成功/失败结算」作为第一关核心闭环，而非单纯靶场或无限波次。
- **结果**：玩法辨识度极高，天然串起多人合作、目标物归属同步、守点防守与切刀跑图撤离等核心工程点。

### ADR-002：将「联机同步与网络优化」定为第一技术亮点
- **背景**：本项目是用户最主打的实习作品，且已与另一项目 `Aura_5_8`（GAS RPG）形成互补；TPS 战斗/客户端岗位高度看重网络工程能力。
- **决策**：设立独立网络优化专项（客户端开火表现先行、`COND_OwnerOnly`/`COND_SkipOwner`、高频射击弹道信道优化、弱网模拟与 `stat net` 量化对比），用实测数据支撑简历与面试。

### ADR-003：在现有 `UEnemyCombatComponent` 基础上做策略化扩展以支撑远程敌人
- **背景**：当前敌人战斗组件已具备完善的服务端 `AttackId` 状态机与 DataAsset 校验，但命中执行写死了近战几何判定。
- **决策**：不推翻重写，而是将“招式执行层”抽离为可扩展策略接口/对象（近战挥砍 vs 远程射击），同时结合 EQS 支撑远程站位与掩体战术，验证框架可扩展性。

### ADR-004：强制推行 Living Documentation（新增功能必同步更新 Markdown）
- **背景**：项目将由 Antigravity 与本地 GPT Agent 双 AI 协作推进，代码高频演进易导致文档失真。
- **决策**：将“每新增一个功能必须同步更新 `Docs/` 对应 Markdown 文档”写入 `AGENTS.md` 硬性规范，确保跨 AI 零漂移与面试复盘素材实时沉淀。

### ADR-005：受击反馈采用「HealthComponent 统一伤害管道 + 独立 HitFeedbackComponent + DataAsset」
- **背景**：需同时覆盖玩家与敌人的受击特效，并兼顾后续爆头倍率、击杀强化特效、Hitmarker 与联机带宽控制。
- **决策**：由 `UHealthComponent` 通过 `OnTakePointDamage`（缓存命中点）+ `OnTakeAnyDamage`（单次扣血消费）统一收口并广播 `OnDamageTaken`；独立的 `UHitFeedbackComponent` 订阅该事件，经服务端频控节流后用 `Unreliable` Multicast（压缩坐标/法线 + 1 字节 `HitFlags`）读取 `UHitFeedbackConfig` DataAsset 播特效。
- **结果**：彻底杜绝双倍扣血，数值与表现零耦合，所有伤害来源自动复用，网络包体极小。
