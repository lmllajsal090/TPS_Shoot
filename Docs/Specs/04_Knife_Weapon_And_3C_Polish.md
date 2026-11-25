# Spec + TDD 04：近战小刀（3号槽）、战斗反馈与 3C 动作打磨

> **状态**：`[Draft — 待核心闭环与联机专项后推进]`
> **所属阶段**：Phase 5 / M5.1 ~ M5.3

---

## 1. 目标与边界（Goal & Non-Goals）

### 目标
1. **新增 3 号武器槽位「小刀（Knife）」**：
   - 将 `UCombatComponent::SwitchWeapon` 中硬编码的 `switch (WeaponType)` 改为**数据驱动**（由武器自身提供切枪 Montage、HideTime、CommitTime 及移速倍率），支持轻松扩展第 3 槽小刀。
   - 实现小刀近战挥砍判定（服务端前向扇形/球体 Sweep）+ **切刀跑图移速加成**（如 `1.2x` 移速，完美贴合类三角洲拿宝撤离体验！）。
2. **极简高收益战斗反馈与续航**：
   - 部位伤害倍率（识别 `head` 骨骼触发爆头 2x 伤害）+ 准星 **Hitmarker**（普通白/爆头红/击杀确认音效）。
   - 简易补给拾取物（弹药包 / 医疗包），避免交火中途断弹或残血无解。
3. **可选 3C 动作扩展**：冲刺（Sprint，收枪/奔跑态）或翻滚闪避（Dodge），视 3 月前时间余量灵活排入。

### 明确不做（Non-Goals）
- 不做复杂连招派生系统、格挡弹反或武器皮肤商店。
- UI 保持极简实用（槽位高亮 1/2/3 + Hitmarker + 任务/撤离条），不堆砌冗余界面。

---

## 2. 武器数据驱动与小刀设计要点

- **扩展 `EWeaponType`**：新增 `Knife`（SlotIndex = 2，快捷键 `3`）。
- **武器配置收口**：在 `AWeaponBase`（或独立 `UWeaponDataAsset`）增加：
  - `SwitchMontage` / `SwitchHideTime` / `SwitchCommitTime`
  - `MoveSpeedMultiplier`（冲锋枪 `1.0`、手枪 `1.05`、小刀 `1.20` — 服务端与客户端同步更新 `CharacterMovement->MaxWalkSpeed`）
  - `bUsesAmmo`（小刀为 `false`，HUD 自动隐藏/切换为近战标识，禁用换弹请求）
- **小刀近战判定**：
  - 复用 `ServerFire` 或专用 `ServerMeleeAttack` 入口，本地先行播放挥刀 Montage + 音效，服务端在命中窗口做前方扫掠检测并调用 `ApplyPointDamage`。

---

## 3. 验收标准（Acceptance Criteria）

- [ ] 快捷键 `1 / 2 / 3` 流畅切换冲锋枪 / 手枪 / 小刀，切刀后跑图速度明显提升且双端移动无回拉抖动（Movement Prediction 正确同步 `MaxWalkSpeed`）
- [ ] 小刀近战挥砍可正常伤害并击杀敌人，无弹药 HUD 错乱
- [ ] 命中敌人触发 Hitmarker，爆头造成更高倍率伤害并有清晰视觉/听觉区分
- [ ] 弹药包与医疗包在双端 PIE 下拾取同步正确
