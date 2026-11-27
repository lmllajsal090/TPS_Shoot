# TPS_Shoot 项目架构快照（基线版本）

> 本文档记录 `TPS_Shoot_Original_5_8` 当前已落地的 C++ 与蓝图架构，作为每次迭代的事实基准。
> **维护约定**：每新增或重构一个功能模块，必须同步更新本文件。

---

## 1. 技术栈与模块配置

- **引擎版本**：Unreal Engine 5.8（从 UE 5.7 升级迁移）
- **主模块**：`TPS_Shoot`（[TPS_Shoot.Build.cs](../Source/TPS_Shoot/TPS_Shoot.Build.cs)）
- **Public 依赖**：`AIModule`, `GameplayTasks`, `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `UMG`, `Slate`, `Niagara`
- **启用插件**：`ModelingToolsEditorMode`, `RetargetAssistant`, `PlayFabParty`（Win64 联机支持）
- **默认关卡与模式**（`Config/DefaultEngine.ini`）：
  - 默认地图：`/Game/ThirdPerson/Lvl_ThirdPerson`
  - 默认 GameMode：`/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode`

---

## 2. 所有权与网络复制拓扑（当前基线）

| 实体 / 组件 | 权威端 | 复制属性 / 同步机制 | 说明与待优化点 |
| :--- | :--- | :--- | :--- |
| **`ATPS_ShootCharacter`** | 服务端（角色移动由 CMC 预测） | `EquippedWeapon`（`OnRep_EquippedWeapon`）、`bIsAiming`（`OnRep_IsAiming`）；RPC：`ServerSwitchWeapon`、`ServerSetAiming`、`ServerReload` | 持有 `UCombatComponent` 与 `UHealthComponent`；广播 `OnEquippedWeaponChanged`、`OnCrosshairShot`、`OnCrosshairReset` |
| **`UCombatComponent`** | 服务端 | `bIsSwitchingWeapon`（Replicated）；RPC：`ServerFire(FVector_NetQuantize)`、`MulticastPlayWeaponSwitch(Reliable)` | 管理 `WeaponSlots`（当前 2 槽：冲锋枪/手枪）、全自动射击定时器、三段定时切枪状态机（Hide -> Commit -> Finish） |
| **`AWeaponBase`** | 服务端 | `CurrentAmmo` / `ReserveAmmo` / `bIsReloading`（`OnRep_WeaponState`）；RPC：`MulticastPlayFireEffects(Unreliable)`、`MulticastPlayImpactDecal(Unreliable)` | **已识别优化点**：弹药目前无条件全端复制（应改 `COND_OwnerOnly`）；客户端开火等整圈 RTT 回播表现（应改本地表现先行）；`SetReplicateMovement(true)` 附着后冗余；每发 Spawn Replicated Projectile Actor |
| **`AAProjectileBase`** | 服务端 | 物理球形碰撞 `OnHit` + `bHasHit` 单次命中守卫 | 命中后通过 `ApplyPointDamage` 扣血并回调 `AWeaponBase::HandleProjectileImpact` 广播弹孔贴花 |
| **`UHealthComponent`** | 服务端 | `MaxHealth` / `CurrentHealth`（`OnRep_HealthValues`）、`bIsDead`（`OnRep_IsDead`） | 挂载在玩家与 `AEnemyBase` 上；通过 `OnTakePointDamage` 缓存 + `OnTakeAnyDamage` 消费实现单次扣血，统一广播 `OnHealthChanged`、`OnDamageTaken(FDamageTakenEvent)` 与 `OnDeath` |
| **`UHitFeedbackComponent` + `UHitFeedbackConfig`** | 服务端触发，客户端表现 | `NetMulticast, Unreliable`（`FVector_NetQuantize` + `FVector_NetQuantizeNormal` + 1 字节 `HitFlags`） | 挂载在玩家与 `AEnemyBase` 上；订阅 `UHealthComponent::OnDamageTaken`，支持 `MinFeedbackInterval` 高频节流（致命一击豁免）、`ENCPoolMethod::AutoRelease` Niagara 池化与音效并发控制 |
| **`AEnemyBase` + `AAEnemyAIController`** | 服务端 | 服务端运行 `BehaviorTree` 与 `UAIPerceptionComponent`（Sight） | 支持受击索敌（`NotifyDamagedBy`）、**多人仇恨防抖**（`TargetSwitchDistanceRatio = 0.8` + `TargetSwitchDistanceAdvantage = 150cm`）、死亡立即 `StopAIOnDeath()` |
| **`UEnemyCombatComponent` + `UEnemyCombatConfig`** | 服务端 | `bIsAttacking`（Replicated）；RPC：`MulticastPlayAttackMontage` / `MulticastStopAttackMontage`（`Unreliable`） | 服务端完整维护前摇-单次命中-后摇-冷却状态机（`CurrentAttackId` 防串台 + 距离/扇形/遮挡三道几何校验）；当前仅实现近战逻辑，下一步抽象扩展远程攻击 |
| **`AATPSHUD` + `UUTPSMainHUD`** | 仅本地客户端 | 无复制，事件驱动 | 监听 `OnPossessedPawnChanged` -> `SetObservedCharacter` -> 订阅武器与血量委托，驱动 `UAmmoWidget`、`UCrosshairWidget`、`UUStateBarWidget` |

---

## 3. 核心业务数据流

### 3.1 玩家射击与伤害流（当前版本）
```text
玩家按下 FireAction (EnhancedInput)
  -> UCombatComponent::StartFire() -> TryFire() [客户端本地校验弹药/换弹与连发间隔]
    -> ServerFire(FVector_NetQuantize AimPoint) [RPC 到服务端]
      -> AWeaponBase::Fire(AimPoint) [服务端校验冷却/弹药 -> 生成 AAProjectileBase -> 扣弹 -> MulticastPlayFireEffects]
        -> AAProjectileBase::OnHit() [单次命中 -> ApplyPointDamage + HandleProjectileImpact]
          -> 目标 UHealthComponent::HandleTakeAnyDamage() [扣血 -> OnRep_HealthValues -> 触发 OnDeath]
          -> 若目标为 AEnemyBase -> NotifyDamagedBy 触发仇恨 / 死亡时 StopAIOnDeath + OnDeathPresentation(溶解)
```

### 3.2 敌人 AI 战斗执行流（当前版本）
```text
AAEnemyAIController (Sight 感知 / NotifyDamagedBy 受击索敌 + 仇恨防抖)
  -> 写入 Blackboard TargetActor -> BT_Enemy 追击
    -> UBTTask_EnemyAttack (独立实例内存 FBTEnemyAttackMemory)
      -> UEnemyCombatComponent::TryStartAttack() [按距离+权重从 UEnemyCombatConfig 选招]
        -> WindupTimer -> HandleAttackHitWindow(AttackId) [距离 + 扇形 + 遮挡校验 -> ApplyPointDamage]
        -> FinishTimer -> HandleAttackComplete(AttackId) [随机冷却偏差 -> 广播 OnAttackFinished]
```

---

## 4. C++ 源码目录索引（`Source/TPS_Shoot/`）

- **根级**：`TPS_Shoot.h/.cpp`、`TPS_ShootCharacter.h/.cpp`、`TPS_ShootGameMode.h/.cpp`、`TPS_ShootPlayerController.h/.cpp`、`EnemyBase.h/.cpp`
- **`AI/`**：`AEnemyAIController.h/.cpp`、`BTTask_EnemyAttack.h/.cpp`
- **`Combat/`**：`CombatComponent.h/.cpp`、`EnemyCombatComponent.h/.cpp`、`EnemyCombatConfig.h/.cpp`
- **`Effect/`**：`HitFeedbackConfig.h/.cpp`、`HitFeedbackComponent.h/.cpp`
- **`Health/`**：`HealthComponent.h/.cpp`
- **`Projectile/`**：`AProjectileBase.h/.cpp`
- **`TPSTarget/`**：`TPSTarget.h/.cpp`（靶标 Actor）
- **`TPSWeapon/`**：`WeaponBase.h/.cpp`
- **`UI/`**：`ATPSHUD.h/.cpp`、`UTPSMainHUD.h/.cpp`、`AMmoWidget.h/.cpp`、`CrosshairWidget.h/.cpp`、`UStateBarWidget.h/.cpp`

---

## 5. 核心蓝图与内容资产索引（`Content/ThirdPerson/`）

- **关卡**：`Lvl_ThirdPerson.umap`（原测试关卡）、`Maps/Lvl_SciFiExtraction.umap`（基于 `ModularScifiHallways` 拼装的三区环线科幻搜打撤大地图，含 6 枢纽/双通道环线/东翼以太之心宝库/北翼撤离大厅/全图 NavMesh）
- **玩家与模式**：`BP_ThirdPersonGameMode`、`BP_ThirdPersonPlayerController`、`BP_TrooperPlayer_Polished`、`ABP_TPS_CoherentAimCalibration`
- **敌人与 AI**：`BP_EnemyBase`、`BP_Whisper`、`ABP_EnemyBase`、`ABP_Whisper`、`AIC_EnemyController`、`BB_Enemy`、`BT_Enemy`、`BTS_UpdateTargetDistance`、`BTT_SetMoveSpeed`、`BTT_WhisperAttack`、`Data/DA_WhisperCombatConfig`
- **UI**：`BP_TpsHUD`、`WBP_MainHUD`、`WBP_AimStar`、`WBP_Ammo`、`WBP_StateWidget`
- **受击反馈资产**：`Content/ThirdPerson/Data/DA_PlayerHitFeedConfig`、`Content/FX/Impact/NS_MechHitImpact`、`NS_MechHitOverload`（及 4 个程序化机甲材质 `M_MechHit_Sparks` / `M_MechHit_ElectricArc` / `M_MechHit_CoreFlash` / `M_MechHit_ScorchSmoke`）
- **其他资源目录**：`BloodPack/`（Niagara 受击血液特效包）、`Weapons/`、`Weapons_Free/`、`M1911/`、`TPS_Trooper/`、`SciFITrooper_Man_03/`、`Whisper/`、`FX/`、`Audio/`、`Target/`
