# Panel / PanelGroup Drag 行为修订

对之前的 PanelGroup 设计进行以下修订：

**Panel 和 PanelGroup 都可以被拖拽。**

但两者代表不同的 Drag Payload：

```cpp
enum class DragPayloadType
{
    Panel,
    PanelGroup
};
```

不要实现两套独立 Dock 系统。

两种 Drag 必须共享：

```text
Drag lifecycle
DockTarget detection
DockSide detection
Dock preview
Floating preview
Mouse capture
Drop commit
DockTree mutation
```

区别只在于：

```text
拖动的 payload 是单个 Panel
还是整个 PanelGroup
```

---

# 1. PanelGroup Drag

PanelGroup 通过 Group 标题栏的可拖动区域进行拖拽。

例如：

```text
+---------------------------------------------+
| Scene | Game | Inspector        drag area X |
+---------------------------------------------+
|                                             |
|               Active Panel                  |
|                                             |
+---------------------------------------------+
```

拖动 Group 时：

```text
Payload = PanelGroup
```

整个 Group：

```text
PanelGroup
├── Scene
├── Game
└── Inspector
```

作为一个整体移动。

Group 中的 Panel 不拆散。

---

# 2. Panel Drag

Panel 通过自己的 Tab 进行拖拽。

例如：

```text
+---------------------------------------------+
| [Scene] [Game] [Inspector]                  |
+---------------------------------------------+
```

拖动：

```text
Game Tab
```

时：

```text
Payload = Panel(Game)
```

只有 `Game` 被移动。

原 Group：

```text
Before:

PanelGroup A
├── Scene
├── Game
└── Inspector
```

拖出 Game 后可能变为：

```text
PanelGroup A
├── Scene
└── Inspector

PanelGroup B
└── Game
```

即：

> 单独拖出的 Panel 必须能够形成新的 PanelGroup。

---

# 3. 不要在 BeginDrag 时立即修改 Group

和 DockTree 一样，拖动过程中不能立即：

```text
从原 Group 移除 Panel
创建新 Group
删除原 Group
Collapse DockNode
改变 placement
```

MouseMove 期间只生成 Preview。

真正的数据修改只能发生在 MouseUp Commit。

例如：

```text
PanelGroup A
├── Scene
├── Game
└── Inspector
```

拖动 Game 时，在 MouseUp 前真实数据仍保持不变。

DragSession 只记录：

```text
sourceGroup = A
panel = Game
sourceTabIndex = 1
```

---

# 4. DragSession

推荐统一为一个 DragSession：

```cpp
enum class DragPayloadType
{
    None,
    Panel,
    PanelGroup
};

struct DragSession
{
    DragPayloadType payloadType;

    PanelID panel;
    PanelGroupID group;

    PanelGroupID sourceGroup;
    int sourceTabIndex = -1;

    DockNodeID sourceDockNode;

    Vec2 pressMousePos;
    Vec2 mousePos;
    Vec2 grabOffset;

    DockNodeID dockTarget;
    DockSide dockSide;

    Rect floatingPreviewRect;
    Rect dockPreviewRect;
};
```

具体字段应适配当前项目已有的数据结构。

不要因为示例而引入重复 ID、裸指针或重复 ownership。

---

# 5. 两种拖拽共享同一个 Drop Result

统一流程：

```text
BeginDrag(payload)
        ↓
UpdateDrag()
        ↓
FindDockTarget()
        ↓
Determine DockSide
        ↓
Preview
        ↓
MouseUp
        ↓
CommitDrop(payload, target)
```

不要写成：

```cpp
UpdatePanelDrag();
UpdatePanelGroupDrag();

CommitPanelDock();
CommitPanelGroupDock();

FindPanelDockTarget();
FindPanelGroupDockTarget();
```

如果它们内部逻辑基本相同。

应该尽量共享：

```cpp
UpdateDrag();
FindDockTarget();
CommitDrop();
```

在 Commit 阶段根据 payload 类型处理来源数据。

---

# 6. Panel 拖到另一个 PanelGroup 的空白标题栏

这是 Panel Drag 与 PanelGroup Drag 最大的行为区别。

当且仅当：

```text
Payload = Panel
DropTarget = PanelGroup 的空白标题栏
```

时：

> Panel 应加入目标 PanelGroup，成为新的 Tab。

例如：

```text
Source Group:

[Scene] [Game]


Target Group:

[Inspector] [Console]
```

拖动 `Game` 到 Target 的空白标题栏后：

```text
Source Group:

[Scene]


Target Group:

[Inspector] [Console] [Game]
```

并将：

```text
Game
```

设为目标 Group 的 Active Panel。

因此：

```text
Panel + Empty Group Header
→ Merge into target PanelGroup
```

普通 Panel 内容区域的 Center 不表示页签合并；它与 Unity 的窗口拖放语义一致，
会为被拖动 Panel 创建新的 Floating PanelGroup。目标标题栏中的已有 Tab 也不是
跨 Group 合并入口；已有 Tab 只用于同一个 Group 内交换排列顺序。

---

# 7. PanelGroup 拖到 Center

当：

```text
Payload = PanelGroup
DropSide = Center
```

时不要自动合并两个 Group。

按照当前需求：

```text
PanelGroup + Center
→ Floating
```

也就是说：

```text
Panel Drag + Center
→ Floating Group

PanelGroup Drag + Center
→ Floating Group
```

这是必须明确区分的行为。

---

# 8. Panel 拖到 Edge

如果：

```text
Payload = Panel
```

并 Drop 到：

```text
Left / Right / Top / Bottom
```

不能直接让一个裸 Panel 成为 DockTree Leaf。

首先创建一个新的 `PanelGroup`：

```text
New PanelGroup
└── dragged Panel
```

然后将这个新 Group Dock 到目标 Edge。

例如：

```text
Before:

Target Group
└── Viewport
```

把 `Inspector` Panel 拖到右侧：

```text
Split
├── Target Group
│   └── Viewport
│
└── New Group
    └── Inspector
```

即：

```text
Panel + Edge
→ Create Group
→ Dock Group
```

---

# 9. Panel 拖到无 DockTarget 区域

如果：

```text
Payload = Panel
DockTarget = none
```

则：

```text
创建一个新的 Floating PanelGroup
```

例如：

```text
PanelGroup A
├── Scene
├── Game
└── Inspector
```

将 `Game` 拖到空白位置：

```text
PanelGroup A
├── Scene
└── Inspector

Floating PanelGroup B
└── Game
```

因此：

```text
Panel + no target
→ New Floating PanelGroup
```

---

# 10. PanelGroup 拖到 Edge

如果：

```text
Payload = PanelGroup
DropSide = Left / Right / Top / Bottom
```

直接移动整个 Group：

```text
PanelGroup
├── Scene
├── Game
└── Inspector
```

Group 内容保持完全不变。

执行：

```text
Remove Group from old placement
→ Collapse old DockNode if needed
→ Dock existing Group into target edge
```

不能创建重复 Group。

---

# 11. PanelGroup 拖到空白区域

如果：

```text
Payload = PanelGroup
DockTarget = none
```

则：

```text
PanelGroup → Floating
```

如果原本已经 Floating：

```text
只更新 floatingRect
```

如果原本 Docked：

```text
从 DockTree 移除
→ Collapse
→ placement = Floating
```

---

# 12. Panel 从 Source Group 移除后的处理

Panel Drop 成功后才从 Source Group 中移除 Panel。

例如：

```text
Source:
[A] [B] [C]
```

拖走 B：

```text
Source:
[A] [C]
```

需要维护 Active Panel。

如果 B 原本是 Active：

优先选择合理的相邻 Tab，例如：

```text
原：
[A] [B*] [C]

移走 B 后：

[A] [C*]
```

或者按照现有项目规则选择前一个 / 后一个。

保持确定性即可。

---

# 13. Source Group 变空

如果 Panel Drag Commit 后导致：

```text
PanelGroup
└── empty
```

必须删除该 Group。

如果该 Group 原本 Docked：

```text
Remove empty PanelGroup
→ Remove empty Leaf
→ Collapse Split if necessary
```

如果原本 Floating：

```text
Remove empty Floating Group
```

不能保留空 PanelGroup。

---

# 14. Panel 在同一个 Group 内的标题栏 Drop

例如：

```text
Group A
├── Scene
├── Game
└── Inspector
```

拖 `Game`，最后 Drop 到同组另一个已有 Tab 时交换二者顺序；Drop 到同组
空白标题栏时保持原顺序。普通内容 Center 不属于标题栏操作，会把 Panel
拆成新的 Floating Group。

空白标题栏的确定行为为：

```text
sourceGroup == targetGroup
&& side == Center

→ Cancel / No-op
```

避免：

```text
remove Panel
→ source Group temporarily empty/collapse
→ target reference invalid
```

同组已有 Tab 命中则通过 `TargetTabIndex` 提交交换，不执行 remove + insert，
避免临时清空 Group 或让 DockTree 的目标引用失效。

---

# 15. Panel Drag Preview

拖动单个 Panel 时，Floating Preview 应表现为一个：

```text
临时单 Panel PanelGroup
```

例如拖动：

```text
Inspector
```

Preview 可以显示：

```text
+----------------------+
| Inspector            |
+----------------------+
|                      |
|   Inspector Preview  |
|                      |
+----------------------+
```

不需要真的创建 PanelGroup。

这只是视觉 Preview。

直到 MouseUp 确认 Floating / Edge Dock 后才真正创建 Group。

---

# 16. PanelGroup Drag Preview

PanelGroup Drag 则应保持 Group 的整体外观：

```text
+--------------------------------+
| Scene | Game | Inspector       |
+--------------------------------+
|                                |
|          Active Content        |
|                                |
+--------------------------------+
```

Preview 应保留：

```text
Group size
Tab layout
grab offset
```

---

# 17. HitTest 优先级

必须明确标题栏中的输入优先级。

推荐：

```text
Group Close Button
        ↓
Panel Tab interaction
        ↓
Panel Tab Drag
        ↓
Group TitleBar Drag
        ↓
Panel Content
```

尤其需要避免：

```text
点击 Tab
→ 同时触发 Group Drag
```

以及：

```text
拖 Panel Tab
→ 被识别为拖整个 Group
```

判断逻辑应基于 MouseDown 时的具体 Hit Region。

---

# 18. Drag Source 在 MouseDown 时确定

例如：

点击：

```text
Panel Tab
```

则：

```text
payloadType = Panel
```

点击：

```text
Group TitleBar Empty Area
```

则：

```text
payloadType = PanelGroup
```

一旦 DragSession 开始：

> Drag payload 类型在本次拖动期间不能改变。

不要因为鼠标后来移动到了不同区域而把：

```text
Panel Drag
```

切换为：

```text
PanelGroup Drag
```

---

# 19. Drop 行为矩阵

必须按照以下矩阵实现：

| Payload    | Center          | Edge                   | No Target                |
| ---------- | --------------- | ---------------------- | ------------------------ |
| Panel      | 创建单 Panel Floating Group | 创建单 Panel Group 后 Dock | 创建单 Panel Floating Group |
| PanelGroup | Floating        | 整个 Group Dock          | 整个 Group Floating        |

其中 Edge 为：

```text
Left
Right
Top
Bottom
```

这是当前需求的核心行为表。

---

# 20. CommitPanelDrop

概念流程：

```cpp
void CommitPanelDrop(const DragSession& drag)
{
    if (IsEmptyGroupHeaderTarget(drag))
    {
        if (drag.sourceGroup == TargetGroup(drag))
            return;

        RemovePanelFromSourceGroup(drag.panel);

        AddPanelToTargetGroup(
            drag.panel,
            TargetGroup(drag));

        SetActivePanel(
            TargetGroup(drag),
            drag.panel);

        CleanupEmptySourceGroup();
        return;
    }

    // 内容 Center、跨 Group 已有 Tab、DockTarget 为空时均进入新建
    // Floating Group 路径；只有 Edge 会把新 Group 放入 DockTree。
    PanelGroupID newGroup =
        CreateGroupContaining(drag.panel);

    RemovePanelFromSourceGroup(drag.panel);

    if (IsEdgeTarget(drag))
    {
        DockGroup(
            newGroup,
            drag.dockTarget,
            drag.dockSide);
    }
    else
    {
        FloatGroup(
            newGroup,
            drag.floatingPreviewRect);
    }

    CleanupEmptySourceGroup();
}
```

这只是行为示意。

实现时必须注意事务顺序和对象 lifetime。

如果删除 Source Group 可能导致 DockTree Collapse，从而让 target handle 失效，应先使用稳定 ID/Handle 保存目标并验证后再 Commit。

---

# 21. CommitPanelGroupDrop

概念流程：

```cpp
void CommitPanelGroupDrop(const DragSession& drag)
{
    PanelGroupID group = drag.group;

    if (IsEdgeTarget(drag))
    {
        RemoveGroupFromCurrentPlacement(group);

        DockGroup(
            group,
            drag.dockTarget,
            drag.dockSide);
    }
    else
    {
        RemoveGroupFromCurrentPlacement(group);

        FloatGroup(
            group,
            drag.floatingPreviewRect);
    }
}
```

其中：

```text
Center
和
No Target
```

都进入 Floating 路径。

---

# 22. 必须保持的核心 ownership

最终数据关系应尽量保持：

```text
Panel
   ↑
   │ belongs to exactly one
   │
PanelGroup
   ↑
   │ either
   ├──────── Dock Leaf
   │
   └──────── Floating Layer
```

不要形成：

```text
Panel
→ DockNode

同时

Panel
→ PanelGroup
```

这种双重布局 ownership。

Dock 系统操作对象应该统一为：

```text
PanelGroup
```

Panel Drag 在 Commit 时只是：

```text
重新组织 PanelGroup membership
+
必要时创建新的 PanelGroup
```

---

# 23. 新的不变量

除原有 Dock invariants 外，还必须满足：

1. 一个 Panel 必须且只能属于一个 PanelGroup。
2. 一个 PanelGroup 可以包含一个或多个 Panel。
3. 不允许长期存在空 PanelGroup。
4. 一个 PanelGroup 只能处于 Docked 或 Floating 之一。
5. DockTree Leaf 中的布局对象是 PanelGroup。
6. 单 Panel Dock 时必须先形成 PanelGroup。
7. Panel Drag Preview 不修改 Source Group。
8. PanelGroup Drag Preview 不修改 DockTree。
9. Panel 只有投到 PanelGroup 空白标题栏时才合并 Group；内容 Center 创建 Floating Group。
10. PanelGroup + Center 不合并 Group。
11. Drag payload 从 MouseDown 确定后，在整个 DragSession 中保持不变。
12. Commit 后不能有 Panel 同时存在于 Source Group 与 Target Group。
13. Group 删除后不能留下 Drag / Hover / Focus 中的失效引用。

---

# 24. 当前非目标

不要因为支持两种 Drag 而实现：

* 跨 Group 的 Tab 插入位置排序；同一 Group 内允许通过拖放交换两个 Tab；
* 多选 Panel Drag；
* 整组 Panel merge；
* PanelGroup Center merge；
* 跨 native window docking；
* Dock layout undo/redo；
* Drag command framework；
* 通用 Drag-and-Drop framework 重构。

如果现有 DragManager 已经足够泛化，复用它。

如果没有，也只做支持：

```text
Panel
PanelGroup
```

所需的最小扩展。

---

# 25. 实现前检查

在修改代码之前，先阅读当前：

* Panel；
* DockNode；
* DockManager；
* DragSession；
* Floating；
* HitTest；
* Panel title/tab rendering；
* Mouse input；
* Close lifecycle。

重点回答：

```text
1. 当前拖动对象是谁？
2. 当前 DockTree Leaf 保存什么？
3. Panel 当前由谁拥有？
4. Floating Rect 当前存在哪里？
5. 哪部分代码区分 title/tab/content HitTest？
```

然后给出最多 6 条实施计划并直接实现。

不要重新设计整个 Dock 架构。

---

# 26. 必须验证的交互

至少验证：

```text
Panel A
→ 从 Group 1 拖到 Group 2 空白标题栏
→ 成为 Group 2 Tab
```

```text
Panel A
→ 拖到 Group 2 内容 Center
→ 创建新的 Floating Group
```

```text
Panel A
→ 拖到 Dock Left
→ 创建新的单 Panel Group
→ 新 Group Dock Left
```

```text
Panel A
→ 拖到空白
→ 创建 Floating Group
```

```text
PanelGroup
→ 拖到 Dock Right
→ 整个 Group Dock Right
```

```text
PanelGroup
→ 拖到 Center
→ Floating
→ 不与目标 Group 合并
```

```text
PanelGroup
→ 拖到空白
→ Floating
```

```text
PanelGroup
→ 拖出窗口客户区
→ 创建独立 Win32 宿主和交换链
→ 保留完整 Group 并在主窗口外完整显示
```

```text
Panel Tab MouseDown
→ Panel Drag

Group TitleBar MouseDown
→ PanelGroup Drag
```

```text
Panel Drag MouseMove
→ Source Group 和 DockTree 均不发生真实修改
```

```text
拖走 Source Group 最后一个 Panel
→ Source Group 被正确删除
→ DockTree 正确 Collapse
```

最终实现应保持：

```text
Panel Drag
和
PanelGroup Drag
```

是：

**同一个 Drag/Dock pipeline 中的两种 payload，而不是两套互相复制的实现。**
