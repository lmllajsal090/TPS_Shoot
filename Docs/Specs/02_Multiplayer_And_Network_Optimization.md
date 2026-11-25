# Spec + TDD 02：联机体验与网络带宽优化专项（第一核心亮点）

> **状态**：`[Draft — 待 Mode A 逐项评审与压测]`
> **所属阶段**：Phase 3 / M3.1 ~ M3.4
> **定位**：本项目最具差异化的工程名片，目标做到 **“发现真实瓶颈 -> 架构级改造 -> 弱网与 `stat net` 量化数据对比”** 完整闭环。

---

## 1. 现状诊断（基于代码审计已识别的 4 个真实联机痛点）

1. **痛点 1 — 客户端开火存在整圈 RTT 手感延迟**：
   - 现状：`UCombatComponent::TryFire` 发送 `ServerFire` 后，客户端必须等待服务端调用 `AWeaponBase::MulticastPlayFireEffects` 回传，才能看到枪口焰、听到枪声、产生后坐力和准星扩张。在 100ms 延迟下射击明显发粘。
2. **痛点 2 — 全自动高频生成 Replicated Projectile Actor**：
   - 现状：冲锋枪每次开火都在服务端 `SpawnActor<AAProjectileBase>`，持续产生大量 Actor Channel 创建/销毁开销与网络抖动。
3. **痛点 3 — 冗余属性复制与附着移动复制**：
   - 现状：`AWeaponBase::CurrentAmmo` 与 `ReserveAmmo` 使用无条件 `DOREPLIFETIME`（全房间广播，其实只有持有者 HUD 需要）；`AWeaponBase` 构造启用了 `SetReplicateMovement(true)`，但在附着到手部 Socket 后并不需要独立移动同步。
4. **痛点 4 — 缺少标准化的联机入口与弱网验证基线**：
   - 现状：仅支持编辑器 PIE 多人启动，尚无独立 Host / Join 会话流程与网络性能基线数据。

---

## 2. 核心优化方案设计（逐项落地 + 量化对比）

### 优化 1：客户端开火表现先行（Client-Side Cosmetic Prediction）
- **方案**：
  - 本地 Owning Client 在 `TryFire()` 本地校验通过时，**立即播放枪口 Niagara、开火音效、施加后坐力与准星扩张**，同时发送 `ServerFire(FVector_NetQuantize AimPoint)`。
  - 服务端校验通过后，广播开火表现时**跳过 Owning Client**（在 `MulticastPlayFireEffects` 内判断 `OwnerCharacter->IsLocallyControlled()` 直接返回，或改用 `COND_SkipOwner` 的 `ReplicatedUsing` 射击计数结构）。
- **收益**：客户端开火反馈延迟从 **1 × RTT（60~150ms）降至 0ms**，且无双播。

### 优化 2：高频射击弹道网络优化（消除 Actor Channel 抖动）
- **方案对比（开工前二选一拍板）**：
  - **方案 A（推荐 · 现代 TPS 标准做法）**：服务端权威射线/弹道校验（或短距离 Hitscan + 弹道 tracer），子弹视觉拖尾（Tracer Niagara / 非复制本地弹丸）仅在各端本地生成，**彻底消除子弹 Replicated Actor**。
  - **方案 B（保留物理弹丸）**：保留实弹但**关闭弹丸复制**，服务端生成权威无渲染弹丸算命中，各客户端同步生成本地表现弹丸（配合对象池复用，零 Spawn/GC 尖峰）。
- **收益**：全自动连续扫射时 Actor Channel 增减量 **降低 90%+**，显著降低带宽峰值与服务端 CPU 开销。

### 优化 3：精准复制条件与 Actor 网络预算调优
- `AWeaponBase::CurrentAmmo`、`ReserveAmmo` 改为 `DOREPLIFETIME_CONDITION(..., COND_OwnerOnly)`。
- 武器装备附着在角色手部后关闭 `SetReplicateMovement(false)`（仅落地拾取物状态需要位置同步）。
- 按实体重要性分层配置 `NetUpdateFrequency`、`MinNetUpdateFrequency` 与 `NetCullDistanceSquared`（玩家高优先级、远处敌人/静态拾取物降频与距离裁剪）。
- 审查全部 `NetMulticast`：确保所有纯表现型 RPC（特效、弹孔、受击动画）均为 `Unreliable`，严禁滥用 `Reliable` 堵塞可靠通道。

### 优化 4：轻量化联机会话子系统（Host / Join）
- 新增 `UTPSSessionSubsystem`（`UGameInstanceSubsystem`），封装局域网 Listen Server / `PlayFabParty` 建房与加入流程（`ServerTravel` / `ClientTravel`），配极简入口界面（Host / Join / 输入 IP 或房间名）。

---

## 3. 弱网压测与量化对比记录表（实现后填入实测数据）

> **标准测试场景**：2 Players Listen Server PIE，模拟弱网 `Net PktLag=80`、`Net PktLagVariance=20`、`Net PktLoss=2`，冲锋枪连续扫射 30 发 + 3 只敌人交火。

| 指标（来源：`stat net` / Unreal Insights） | 优化前（Baseline） | 优化后（Optimized） | 改善幅度 |
| :--- | :--- | :--- | :--- |
| 客户端开火反馈延迟 | 待测（约 1 RTT） | 待测（目标 0ms） | - |
| 扫射期间 Outgoing 带宽（KB/s） | 待测 | 待测 | - |
| 扫射期间 Active Actor Channels 峰值 | 待测 | 待测 | - |
| 弱网下切枪/换弹/撤离状态一致性 | 待测 | 待测 | - |

---

## 4. 验收标准（Acceptance Criteria）

- [ ] 弱网模拟（80ms 延迟 + 2% 丢包）下客户端开火无粘滞、无双播枪声/后坐力
- [ ] 完成优化前后 `stat net` 对比数据采集并更新至本表与 `README.md`
- [ ] 双端可通过 Host / Join 顺利进入同一局搜打撤并完成完整对局
