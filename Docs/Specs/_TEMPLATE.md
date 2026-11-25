# Spec + TDD 模板：[模块名称]

> **状态**：`[Draft | In Review | Approved | Implemented]`
> **所属阶段**：Phase X / M X.X
> **文档维护提醒**：本功能实现并验证后，务必同步更新 `Docs/01_Architecture.md`、`Docs/02_Roadmap.md` 与 `Docs/03_Progress_Log.md`。

---

## 1. 目标与边界（Goal & Non-Goals）

### 目标（What we build）
- 解决什么玩法、联机或架构问题？
- 面试时希望用该模块证明什么工程能力？

### 明确不做（Non-Goals）
- 明确裁剪项，防止偏离 3 月截止日期。

---

## 2. 玩法与状态流（Flow & State Machine）

```mermaid
flowchart LR
    A[初始状态] --> B[核心交互/触发]
    B --> C[服务端权威校验]
    C --> D[状态同步与客户端表现]
```

关键边界情况（Edge Cases）：
- 玩家掉线/死亡、多人同时交互竞争、网络延迟乱序到达。

---

## 3. C++ 与蓝图职责切分

| 层级 | 负责内容 |
| :--- | :--- |
| **C++** | 核心状态机、数据结构、服务端校验、网络复制/RPC、性能优化 |
| **Blueprint / Asset** | 资产装配、粒子/音效/动画表现、UMG 布局样式、关卡摆放 |

---

## 4. 数据结构与类接口设计

- 新增 / 修改的类、组件、DataAsset 与核心公开方法签名
- 委托（Delegate）与解耦方式

---

## 5. 联机同步与网络优化方案（Replication & Budget）

- **权威端**：哪些判定仅限 `HasAuthority()`？
- **复制属性与条件**：明确 `COND_None` / `COND_OwnerOnly` / `COND_SkipOwner`、`ReplicatedUsing` 与 `NetUpdateFrequency`
- **RPC 选型**：明确 `Server/Client/NetMulticast` 及 `Reliable` vs `Unreliable` 理由
- **带宽/延迟考量**：是否存在高频发包风险？如何做本地表现先行或量化压缩？

---

## 6. 验收标准（Acceptance Criteria）

- [ ] 单人 PIE 下完整跑通，无 Warning / Error
- [ ] Listen Server + Client（至少 2 Players）在弱网模拟（`Net PktLag=60 PktLoss=2`）下状态一致、手感流畅
- [ ] UBT 编译 0 Error，且已同步更新相关 Markdown 文档
