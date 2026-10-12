# 原 TPS 学习项目的 UE 5.8 副本

日期：2026-10-12。引擎：UE 5.8.3。

## 打开

- UE：双击本目录 `OpenInUE58.cmd`，或 `TPS_Shoot.uproject`。
- Rider：打开本目录 `TPS_Shoot.uproject` 或新生成的 `TPS_Shoot.sln`。
- 默认地图：`/Game/ThirdPerson/Lvl_ThirdPerson`。

来源为 `C:\Users\lesoci\Documents\Unreal Projects\TPS_Shoot` 的当前已保存内容，包含原项目已有的输入与C++修改。该来源仍关联5.7，迁移后逐文件校验和Git状态均保持一致。

## 迁移内容

项目关联改为5.8，两个Target改为V7 / Unreal5_8；游戏模块的9个源码与构建文件和原项目一致。239个资源重新保存为5.8版本；一个手枪Aim动画原有的失效RetargetSourceAsset引用和预览网格设置改为同Skeleton的SKM_Quinn_Simple，没有编辑动画关键帧。

## 验证

- 原项目258个文件哈希与原Git状态未变化。
- C++ Editor目标编译成功，新IDE工程生成成功。
- 13个蓝图、界面、接口和Control Rig编译：0错误、0警告、0加载失败。
- 239/239资源转换成功；原预览引用警告已定位并修正。
- 新副本地图已在UE5.8编辑器打开，PIE角色与动画实例正常生成；通过Pawn.AddMovementInput验证实际位移，随后结束PIE并返回编辑器。

验证数据：`迁移验证.json`。日志：`Saved/Workflow`。本轮没有验证实际键鼠操作、打包版或原项目的全部玩法。

此目录是原学习工程的升级副本。你研究新增四枪、AI、背包和IK的演示源码时，使用 `C:\UEProjects\TPS_Shoot_Demo` 及其学习导读。

## Git 仓库

本副本有独立的 `.git` 目录，保留原项目的提交历史，远程 `origin` 为 https://github.com/lmllajsal090/TPS_Shoot.git 。当前本地分支为 `upgrade/ue5.8`，尚未提交或推送迁移改动，首次发布此分支后再设置远程跟踪。

原项目仍在原位置使用 `main` 分支。本副本的资源转换、Target 更新及原有未提交修改都保留在工作区；可以在 Rider 的 Git 改动列表中检查。生成的 `.sln`、`.slnx`、缓存和编译产物已忽略。
