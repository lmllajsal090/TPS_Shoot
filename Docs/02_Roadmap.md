# TPS_Shoot 开发路线图（3 月前旗舰 Demo 计划）

> **核心定位**：可联机 TPS 类《三角洲行动》**搜-打-撤**演示项目（个人最主打作品）。
> **技术优先级**：**1. 联机同步与网络优化** > **2. 高扩展性敌人框架（近战 + 远程）** > **3. 搜打撤完整闭环（非洲之星 + 撤离点）** > **4. 小刀 3 号槽 / 极简实用 UI / 3C 动作打磨**。
> **状态说明**：`[x]` 已完成 / `[-]` 进行中 / `[ ]` 待开始

---

### Phase 1：基础 TPS 战斗与工程底座（已完成）
- [x] UE 5.8 迁移与验证、冲锋枪/手枪双槽切换、ADS 开镜、后坐力、动态准星、弹孔贴花
- [x] `UHealthComponent` 复制血量、平滑插值血条、敌人死亡溶解
- [x] 敌人近战战斗架构（`AEnemyBase`、`AAEnemyAIController` 仇恨防抖、`UEnemyCombatComponent`、`UEnemyCombatConfig`、`UBTTask_EnemyAttack`）
- [x] 建立 GitHub 级项目文档体系（`README.md` + `AGENTS.md` + `Docs/Specs/` + Living Docs 规范）

---

### Phase 2：类三角洲「搜-打-撤」核心玩法闭环
> 详细设计见：[Docs/Specs/01_Extraction_Loop.md](Specs/01_Extraction_Loop.md)

- [ ] **M2.1 对局阶段机与 GameState 同步**
  - 新增 `ATPS_ShootGameState`（继承 `AGameStateBase`）同步对局阶段（`Exploring` -> `CountdownActive` -> `Extracting` -> `Extracted` / `Failed`）、目标持有人与时间戳倒计时
  - 完善 `ATPS_ShootGameMode`：全队阵亡/超时失败判定、撤离成功结算、断线掉落兜底与一键重开流程
- [ ] **M2.2 核心高价值目标物：「以太之心（Aether Core）」**
  - `AAetherCoreActor`：支持 `E` 键交互拾取（`ServerInteract` 原子校验防抢拾竞态）、拾取后触发全局倒计时、持有人阵亡/断线自动在原地贴地掉落
- [ ] **M2.3 撤离点系统与极简撤离 UI**
  - `AExtractionZone`：校验持有人进入撤离区 -> 启动服务端权威撤离读条（可配置时长）-> 读条归零全队撤离胜利
  - 极简 HUD 扩展：任务目标提示条 + 全局倒计时/撤离进度条 + 胜利/失败结算弹窗（v2 再加撤离期间敌人加强/增援）

---

### Phase 3：第一核心亮点 — 联机体验与网络优化专项
> 详细设计见：[Docs/Specs/02_Multiplayer_And_Network_Optimization.md](Specs/02_Multiplayer_And_Network_Optimization.md)

- [ ] **M3.1 客户端开火表现先行（消除 RTT 射击延迟）**
  - 本地客户端开火即刻触发枪口特效/音效/后坐力/准星扩张，服务端通知跳过 Owning Client，实现弱网下开火零粘滞感
- [ ] **M3.2 带宽与复制专项优化**
  - 精准复制条件：`CurrentAmmo` / `ReserveAmmo` 改为 `COND_OwnerOnly`；梳理不必要的高频同步字段
  - 武器附着优化：消除武器附着在角色手部后冗余的 `SetReplicateMovement(true)` 同步开销
  - 高频射击弹道优化：消除全自动冲锋枪每发 Spawn Replicated Projectile Actor 带来的 Actor Channel 抖动（采用客户端表现弹道 + 服务端权威射线/弹道校验，或轻量弹道池方案）
  - Actor 网络裁剪与更新频率调优（`NetUpdateFrequency` / `MinNetUpdateFrequency` / `NetCullDistanceSquared`）
- [ ] **M3.3 弱网量化压测报告**
  - 在 2~3 人 Listen Server PIE 下开启弱网模拟（如 `Net PktLag=80 PktLagVariance=20 PktLoss=2`），用 `stat net` / Network Insights 记录优化前后数据并写入文档与 `README.md`
- [ ] **M3.4 联机建房与加入入口**
  - 搭建轻量联机会话子系统（`UGameInstanceSubsystem`，支持局域网 Listen Server / `PlayFabParty` 建房与加入）及极简 Host / Join 入口界面

---

### Phase 4：第二核心亮点 — 高扩展性敌人框架与远程敌人
> 详细设计见：[Docs/Specs/03_Extensible_Enemy_And_Ranged_AI.md](Specs/03_Extensible_Enemy_And_Ranged_AI.md)

- [ ] **M4.1 敌人战斗框架策略化重构**
  - 保留 `UEnemyCombatComponent` 权威状态机与 `UBTTask_EnemyAttack` 不变，将招式执行层抽象为可扩展策略（如多态 `UEnemyAttackAction` 或 `Melee / Ranged` 清晰分离），保证新增敌人类型无需修改 `AEnemyBase` 或行为树节点
- [ ] **M4.2 新增远程射击敌人 + EQS 战术站位**
  - 复用 `AEnemyBase` + `AAEnemyAIController`，配置远程 DataAsset（射程、点射间隔、弹道散布、预警表现）
  - 接入 UE **EQS（Environment Query System）**：远程敌人自动寻找带视线且保持理想交火距离/靠近掩体的战术位置，与近战 `Whisper` 形成包抄压制配合

---

### Phase 5：武器闭环（小刀）、手感反馈与关卡收口
> 详细设计见：[Docs/Specs/04_Knife_Weapon_And_3C_Polish.md](Specs/04_Knife_Weapon_And_3C_Polish.md)

- [ ] **M5.1 近战小刀（Slot 3）与切枪数据驱动化**
  - 重构 `UCombatComponent::SwitchWeapon` 硬编码 `switch`，改为由武器配置驱动切枪 Montage 与时间节点
  - 新增 3 号槽位「小刀（Knife）」：近战扇形/胶囊挥砍判定 + **切刀跑图移速加成**（贴合三角洲跑图撤离手感）
- [ ] **M5.2 极简高收益战斗反馈与续航**
  - [x] 受击反馈系统（`UHealthComponent` 统一伤害事件 + `UHitFeedbackComponent` + `UHitFeedbackConfig` DataAsset + 科幻机甲定制受击特效，见 [05_Hit_Feedback_System.md](Specs/05_Hit_Feedback_System.md)）
  - [ ] 爆头部位伤害倍率（`Hit.BoneName`）+ **Hitmarker 准星命中/爆头/击杀反馈**
  - [ ] 简易弹药/医疗补给拾取点（保证搜打撤全程续航不卡弹）
  - [ ] 角色 3C 机动动作（视进度加冲刺 Sprint / 翻滚闪避 Dodge）
- [ ] **M5.3 第一关动线包装与双端演示录屏**
  - 在关卡中明确布置：**出生点 -> 交火掩体区 -> 高价值藏宝点（非洲之星，近战+远程敌人守卫）-> 撤离点**
  - 完成双端联机实机录屏，整理 `README.md` 展示区

---

### 工程卫生小清理（按需穿插）
- [ ] 清理 `Config/DefaultEngine.ini` 重复配置行与旧的 `F:/TPS/MCPRepair/start_unreal_mcp.py` 路径
- [x] 为仓库补齐 `.gitattributes`（Git LFS 托管 `.uasset` / `.umap`）
- [ ] 统一 `UI/AMmoWidget.h` 文件名大小写为 `AmmoWidget.h`
