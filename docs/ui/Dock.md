# DX12 UI Dock 系统设计与实现约束

请按照以下设计原则实现和维护 Panel Docking 系统。

## 1. 核心设计原则

Dock 系统的核心不是直接修改 Panel 的 `Rect`，而是：

**通过修改 DockTree 的结构描述布局，然后由 Layout 系统根据 DockTree 重新计算所有 Panel 的最终 Rect。**

Panel 不应该自己决定 Dock 后的位置。

整体数据流应保持：

```text
Input
  ↓
HitTest
  ↓
Panel Drag State
  ↓
Dock Candidate / Floating Candidate
  ↓
Dock Transaction
  ↓
DockTree
  ↓
Dock Layout
  ↓
Panel Rect
  ↓
UI DrawList
  ↓
DX12 Renderer
```

DX12 Renderer 只负责渲染最终 Draw Commands，不允许参与 Dock、Drag、HitTest 或布局决策。

---

# 2. DockTree 数据模型

DockTree 应由两种主要节点组成：

```cpp
enum class DockNodeType
{
    Leaf,
    Split
};

enum class SplitAxis
{
    Horizontal, // 上下
    Vertical    // 左右
};
```

推荐基础结构：

```cpp
struct DockNode
{
    DockNodeID id;

    DockNodeType type;

    DockNode* parent = nullptr;

    // Layout calculation result
    Rect rect;

    // Split node
    SplitAxis splitAxis;
    float splitRatio = 0.5f;

    DockNode* childA = nullptr;
    DockNode* childB = nullptr;

    // Leaf node
    std::vector<Panel*> panels;
    int activePanel = 0;
};
```

其中：

```cpp
rect
```

只是 Layout 计算结果，不是布局的 source of truth。

真正决定布局的数据是：

```cpp
splitAxis
splitRatio
childA
childB
DockTree hierarchy
```

---

# 3. Leaf 与 Split 的职责

## Leaf Node

Leaf 表示真正能够容纳 Panel 的 Dock 区域。

一个 Leaf 可以包含多个 Panel，并形成 Tab：

```text
Leaf
├── Scene
├── Game
└── Inspector
```

其中：

```cpp
activePanel
```

决定当前显示哪个 Panel。

---

## Split Node

Split 本身不直接拥有 Panel。

它只负责把自己的矩形区域切成两个区域：

```text
Split
├── childA
└── childB
```

例如：

```text
Split Vertical
├── Hierarchy
└── Viewport
```

表示：

```text
+-------------+----------------------+
| Hierarchy   |      Viewport        |
|             |                      |
+-------------+----------------------+
```

---

# 4. Dock Layout

Dock Layout 必须从 Root 开始递归计算。

伪代码：

```cpp
void LayoutNode(DockNode* node, const Rect& rect)
{
    node->rect = rect;

    if (node->type == DockNodeType::Leaf)
        return;

    if (node->splitAxis == SplitAxis::Vertical)
    {
        float widthA = rect.width * node->splitRatio;

        Rect rectA = {
            rect.x,
            rect.y,
            widthA,
            rect.height
        };

        Rect rectB = {
            rect.x + widthA,
            rect.y,
            rect.width - widthA,
            rect.height
        };

        LayoutNode(node->childA, rectA);
        LayoutNode(node->childB, rectB);
    }
    else
    {
        float heightA = rect.height * node->splitRatio;

        Rect rectA = {
            rect.x,
            rect.y,
            rect.width,
            heightA
        };

        Rect rectB = {
            rect.x,
            rect.y + heightA,
            rect.width,
            rect.height - heightA
        };

        LayoutNode(node->childA, rectA);
        LayoutNode(node->childB, rectB);
    }
}
```

每一帧或者 DockTree / Workspace 尺寸改变后：

```cpp
LayoutNode(root, workspaceRect);
```

所有 Docked Panel 的 Rect 都必须由 DockNode 的最终 Rect 推导得到。

不要直接修改 Panel Rect 来完成 Dock。

---

# 5. Dock 操作本质是修改树结构

Dock：

```cpp
DockPanel(
    Panel* panel,
    DockNode* target,
    DockSide side
);
```

其中：

```cpp
enum class DockSide
{
    Left,
    Right,
    Top,
    Bottom,
    Center
};
```

---

# 6. Left / Right / Top / Bottom Dock

假设目标节点：

```text
Target
└── Viewport
```

Panel Dock 到 Target 左边时：

```text
之前：

Viewport
```

修改为：

```text
Split Vertical
├── NewPanel
└── Viewport
```

Right：

```text
Split Vertical
├── Viewport
└── NewPanel
```

Top：

```text
Split Horizontal
├── NewPanel
└── Viewport
```

Bottom：

```text
Split Horizontal
├── Viewport
└── NewPanel
```

实现时应该创建新的 Split Node 和 Leaf Node，并将旧 Target 替换为新的 Split。

不要直接修改两个 Panel 的 Rect。

---

# 7. Center Dock

以下是 DockTree 原始 Leaf 模型的基础语义。当前 UI 架构已由
`PanelGroupNode` 显式拥有页签，DockTree Leaf 只保存 Group，因此交互层不会再把
普通 Center 直接提交为隐式 Tab：Panel 内容 Center 创建 Floating Group，只有
PanelGroup 的空白标题栏会在控件所有权层合入 Panel；PanelGroup Center 整体浮动。

Center Dock 不应该创建 Split。

Center 表示将 Panel 插入目标 Leaf：

```text
Leaf
├── Viewport
└── Scene
```

形成 Tab。

因此：

```cpp
if (side == DockSide::Center)
{
    targetLeaf->panels.push_back(panel);
}
```

Left / Right / Top / Bottom：

```text
修改 DockTree 结构
```

Center：

```text
修改 Leaf 的 Tab 列表
```

两者必须严格区分。

---

# 8. Splitter Resize

用户拖动两个 Dock 区域之间的 splitter 时，不要直接修改两个 Child Rect。

只修改：

```cpp
splitRatio
```

例如：

```cpp
splitRatio =
    (mouseX - node.rect.x)
    / node.rect.width;
```

然后 clamp：

```cpp
splitRatio = std::clamp(
    splitRatio,
    minRatio,
    maxRatio);
```

再重新执行：

```cpp
LayoutNode(root, workspaceRect);
```

Rect 永远应该由 Layout 推导。

---

# 9. Empty Node Collapse

如果一个 Leaf 的最后一个 Panel 被移走：

```text
Split
├── Empty
└── Viewport
```

不能保留 Empty Leaf。

应该 Collapse：

```text
Viewport
```

也就是说：

如果 Split 只剩一个有效 Child：

```text
删除 Empty Leaf
删除 Split
用 sibling 替换 Split
```

Root 也必须正确处理。

这一逻辑应该由统一函数负责，例如：

```cpp
CollapseEmptyNode();
```

不要把 Collapse 逻辑散落在不同 Dock 操作中。

---

# 10. Floating Panel 与 DockTree 分离

Floating Panel 不应该成为 DockTree 的一种节点。

推荐结构：

```cpp
struct UIContext
{
    DockNode* dockRoot;

    std::vector<Panel*> floatingPanels;
};
```

Panel 可以具有：

```cpp
enum class PanelPlacement
{
    Docked,
    Floating
};
```

例如：

```cpp
struct Panel
{
    PanelID id;

    PanelPlacement placement;

    DockNode* dockNode = nullptr;

    Rect floatingRect;
};
```

Docked Panel：

```text
位置由 DockTree 决定
```

Floating Panel：

```text
位置由 floatingRect 决定
```

---

# 11. Panel Drag 的核心原则

一个 Panel 一般只能通过：

```text
TitleBar
或者 Tab
```

开始拖动。

但是：

**开始拖动并不意味着立即切换成 Floating。**

Dragging 是一个独立的临时状态。

Panel 最终是 Floating 还是 Docked，应该在 MouseUp 时根据当前 Drop Target 决定。

---

# 12. Panel Drag State

推荐实现：

```cpp
enum class PanelDragState
{
    Idle,
    Pressed,
    Dragging
};
```

MouseDown TitleBar：

```text
Idle
 ↓
Pressed
```

鼠标移动超过 threshold：

```text
Pressed
 ↓
Dragging
```

例如：

```cpp
if (Length(mousePos - pressPos) > 5.0f)
{
    BeginDragging();
}
```

threshold 用于避免普通点击被识别成拖动。

---

# 13. DragSession

拖动信息必须集中保存，不要把临时拖动状态分散到 Panel / DockNode / Input 等多个对象。

推荐：

```cpp
struct PanelDragSession
{
    Panel* panel = nullptr;

    // Source
    DockNode* sourceNode = nullptr;
    int sourceTabIndex = -1;

    bool sourceWasFloating = false;
    Rect sourceFloatingRect;

    // Mouse
    Vec2 pressMousePos;
    Vec2 mousePos;
    Vec2 grabOffset;

    // Preview
    Rect floatingPreviewRect;

    // Dock candidate
    DockNode* dockTarget = nullptr;
    DockSide dockSide;

    Rect dockPreviewRect;
};
```

---

# 14. Dragging 不等于 Floating

不要这样做：

```cpp
BeginDrag()
{
    panel->placement = Floating;
}
```

这是错误的设计。

正确流程：

```text
MouseDown
   ↓
Pressed
   ↓
Dragging
   ↓
不断寻找 Drop Target
   ↓
MouseUp
   ↓
Commit
```

在整个 Dragging 阶段：

Panel 当前的最终归属还没有确定。

---

# 15. Dock Candidate 与 Floating Candidate

Dragging 每一帧进行 HitTest。

如果鼠标位于有效 Dock 区域：

```cpp
drag.dockTarget != nullptr;
```

则当前是：

```text
Dock Candidate
```

并计算：

```cpp
dockSide
dockPreviewRect
```

如果：

```cpp
drag.dockTarget == nullptr;
```

则当前是：

```text
Floating Candidate
```

因此不需要把：

```text
Floating Drag
Dock Drag
```

实现成两套完全不同的状态机。

它们只是同一个 Drag Session 的两个候选结果。

---

# 16. MouseUp 时才决定 Dock / Floating

核心逻辑：

```cpp
void EndPanelDrag()
{
    if (drag.dockTarget != nullptr)
    {
        CommitDock(drag);
    }
    else
    {
        CommitFloating(drag);
    }
}
```

规则必须保持：

```text
MouseUp + valid DockTarget
→ Dock

MouseUp + no DockTarget
→ Floating
```

---

# 17. Drag 过程中不要修改 DockTree

这是非常重要的约束。

MouseMove 阶段：

```text
只允许计算：
- hovered DockNode
- DockSide
- DockPreviewRect
- FloatingPreviewRect
```

不允许：

```text
RemovePanelFromDock
InsertDockNode
CollapseDockNode
重新组织 DockTree
```

否则可能出现：

```text
拖动开始
↓
Panel 被立刻移出 DockTree
↓
Source Leaf 变空
↓
Collapse
↓
Layout 改变
↓
鼠标下面的 DockNode 改变
↓
HitTest 改变
↓
Dock Preview 跳动
```

因此：

**Drag = Preview**

**MouseUp = Commit**

---

# 18. Floating Preview

Floating Preview 通常跟随鼠标。

必须保存：

```cpp
grabOffset =
    mouseDownPos - panelRect.position;
```

然后：

```cpp
floatingPreview.position =
    mousePos - grabOffset;
```

这样开始拖动时 Panel 不会突然将左上角吸附到鼠标。

---

# 19. Dock Target Detection

每帧：

```cpp
DockNode* FindDockTarget(Vec2 mouse);
```

找到当前鼠标所在的合法 Leaf / DockNode。

然后：

```cpp
DockSide DetectDockSide(
    const Rect& targetRect,
    Vec2 mouse);
```

第一版可以使用简单区域：

```text
         TOP

LEFT   CENTER   RIGHT

        BOTTOM
```

例如：

```cpp
float nx =
    (mouse.x - rect.x) / rect.width;

float ny =
    (mouse.y - rect.y) / rect.height;

if (nx < 0.25f)
    Left;

else if (nx > 0.75f)
    Right;

else if (ny < 0.25f)
    Top;

else if (ny > 0.75f)
    Bottom;

else
    Center;
```

以后可以替换成 IDE 风格 Dock Icon，但底层 Dock 逻辑不需要改变。

---

# 20. Dock Preview

Dock Candidate 出现后，仅绘制 Preview。

例如：

```text
Viewport
+-----------------------------+
|                    |////////|
|                    |////////|
|                    |////////|
+-----------------------------+
```

Preview 不应该真正修改 Viewport Rect。

推荐 API：

```cpp
Rect GetDockPreviewRect(
    const Rect& targetRect,
    DockSide side);
```

---

# 21. CommitFloating

如果 Panel 原本 Docked：

```text
Remove Panel From Dock Leaf
↓
如果 Leaf empty
↓
Collapse Empty Node
↓
Panel placement = Floating
↓
设置 floatingRect
```

如果 Panel 原本已经 Floating：

```text
只更新 floatingRect
```

---

# 22. CommitDock

无论来源是 Floating 还是 Docked，都通过统一逻辑处理。

例如：

```cpp
void CommitDock(PanelDragSession& drag)
{
    RemovePanelFromCurrentPlacement(drag.panel);

    DockPanel(
        drag.panel,
        drag.dockTarget,
        drag.dockSide);
}
```

但必须注意：

在真正修改 DockTree 前，先保存 Target。

避免 Source 被删除 / Collapse 后导致 Target 指针失效。

最好最终使用：

```cpp
DockNodeID
```

而不是长期依赖裸指针。

---

# 23. DockTransaction

推荐将一次 Dock 操作抽象成 Transaction：

```cpp
struct DockTransaction
{
    PanelID panel;

    DockNodeID sourceNode;

    DockNodeID targetNode;

    DockSide targetSide;

    bool targetFloating;

    Rect floatingRect;
};
```

流程：

```text
Generate Transaction
↓
Validate Transaction
↓
Commit Transaction
↓
Validate DockTree
↓
Layout
```

不要一边 HitTest 一边直接修改 DockTree。

---

# 24. Dock Invariants

实现时必须维持以下不变量。

### Invariant 1

一个 Panel 必须且只能处于：

```text
Docked
或
Floating
```

之一。

不能同时存在。

---

### Invariant 2

Docked Panel 必须属于且只属于一个 Leaf。

---

### Invariant 3

Split Node 必须具有两个有效 Child。

不能长期存在：

```text
Split
├── child
└── null
```

---

### Invariant 4

Empty Leaf 应及时 Collapse。

---

### Invariant 5

Sibling Rect 不允许非法 overlap。

---

### Invariant 6

Child Rect 必须完整覆盖 Parent 对应的 layout region。

---

### Invariant 7

`splitRatio` 必须保持合法范围：

```cpp
minRatio <= splitRatio <= maxRatio;
```

---

### Invariant 8

Drag Preview 不允许修改 DockTree。

---

### Invariant 9

只有 Commit 阶段允许结构性修改 DockTree。

---

### Invariant 10

同样的：

```text
DockTree
+
Root Rect
```

必须得到 deterministic 的 Layout 结果。

---

# 25. 坐标系要求

Dock Drag / HitTest 中所有坐标必须明确属于哪个空间。

不要只使用含义模糊的：

```cpp
Vec2 mousePos;
```

需要明确区分：

```text
Screen
Client
UI
Panel Local
```

例如：

```cpp
ClientPoint mouseClient;
UIPoint mouseUI;
```

DockNode Rect 和 Mouse HitTest 必须处于同一个坐标空间。

禁止直接比较不同坐标空间的数据。

---

# 26. Input Capture

Panel Drag 开始以后，需要保证鼠标即使移出窗口仍能够正确结束 Drag。

Win32 Platform Layer 应负责：

```text
Mouse Capture
Mouse Release
Capture Lost
Cancel Drag
```

Dock 逻辑不应该直接依赖 DX12。

---

# 27. Debug 能力

Dock 系统必须能够输出 DockTree。

例如：

```text
Root #1
Split Vertical ratio=0.30

├── Leaf #2
│   Panels:
│   - Hierarchy
│
└── Split #3 Horizontal ratio=0.70
    ├── Leaf #4
    │   Panels:
    │   - Viewport
    │
    └── Leaf #5
        Panels:
        - Console
```

每次结构修改前后都应该能够记录：

```text
DockTree BEFORE
Dock Transaction
DockTree AFTER
```

Drag Debug 至少应包含：

```text
Panel
Source Node
Mouse Position
Hovered Node
Dock Target
Dock Side
Preview Rect
Drag State
```

---

# 28. 推荐模块划分

推荐保持以下职责：

```text
Panel
    Panel 本身的生命周期与内容

DockNode / DockTree
    Dock 数据结构

DockLayout
    根据 DockTree 计算 Rect

DockTransaction
    Dock / Undock / Move / Collapse

DockHitTest
    查找 Dock Target

PanelDragManager
    管理 DragSession

Input
    Mouse / Keyboard / Capture

UIPainter
    绘制 Panel / Tab / Dock Preview

DX12UIRenderer
    绘制最终 DrawList

DockDebug
    DockTree / Transaction / Drag 调试
```

禁止让一个大型 `PanelManager` 同时负责：

```text
Mouse Input
DockTree
Layout
Drag
Rendering
```

---

# 29. 第一阶段实现范围

请不要一开始实现完整 Visual Studio 风格 Dock。

第一阶段只实现：

```text
1. DockRoot

2. Leaf Node

3. Split Node

4. Dock Left

5. Dock Right

6. Dock Top

7. Dock Bottom

8. Dock Center / Tabs

9. Splitter Resize

10. Remove Panel

11. Empty Node Collapse

12. Floating Panel

13. TitleBar Drag

14. Dock Preview

15. Floating Preview

16. MouseUp Commit
```

---

# 30. 推荐开发顺序

必须优先确保 DockTree 正确，再实现鼠标交互。

首先通过代码直接执行：

```cpp
DockPanel(
    hierarchy,
    viewport,
    DockSide::Left);

DockPanel(
    console,
    viewport,
    DockSide::Bottom);
```

验证是否稳定得到：

```text
+------------+----------------------+
|            |                      |
| Hierarchy  |       Viewport       |
|            |                      |
|            +----------------------+
|            |       Console        |
+------------+----------------------+
```

确认以下部分完全正确：

```text
DockTree
Dock Transaction
Collapse
Layout
Splitter
Tabs
```

之后再加入：

```text
Mouse Input
HitTest
Drag
Preview
Mouse Capture
```

不要同时调试所有层。

---

# 31. Codex 调试规则

当 Dock 行为出现问题时，不要直接根据视觉现象修改代码。

必须按照以下顺序检查：

```text
Input
↓
Mouse Coordinate
↓
HitTest
↓
DragSession
↓
Dock Candidate
↓
DockTransaction
↓
DockTree
↓
Layout
↓
Panel Rect
↓
DrawList
↓
DX12 Rendering
```

必须首先找到：

**第一个开始出现错误状态的阶段。**

不要从最终截图反向猜测。

---

# 32. 对 Codex 的具体要求

在修改 Dock 系统之前：

1. 阅读当前 DockNode、Panel、Input、Drag、Layout 相关代码。
2. 建立当前实现的数据流。
3. 判断当前实现是否违反上述职责边界。
4. 找出所有直接修改 Docked Panel Rect 的地方。
5. 找出 Drag 阶段修改 DockTree 的地方。
6. 找出 Dock / Floating 使用两套独立 Drag 状态机的地方。
7. 找出 Empty DockNode 无法正确 Collapse 的地方。
8. 找出 Mouse / Dock Rect 使用不同坐标空间的地方。
9. 找出 Panel 同时可能属于 Floating 和 DockNode 的地方。
10. 找出 Split Child / Parent 关系可能失效的地方。

在确定问题之前不要重写整个系统。

优先进行最小、结构清晰、可以验证的修改。

每次结构修改后验证 DockTree invariants。

---

# 33. 最终目标

Dock 系统最终应该满足：

```text
Panel Drag
不是修改 Rect

Dock
不是修改 Rect

Undock
不是修改 Rect

Splitter Resize
不是直接修改 Child Rect
```

所有布局最终统一为：

```text
修改 Dock 数据
↓
DockTree
↓
Layout
↓
Rect
↓
Rendering
```

而 Panel Drag 最终统一为：

```text
Begin Drag
↓
Update Preview
↓
Determine Drop Candidate
↓
MouseUp
↓
Commit Dock / Floating
```

其中：

**Floating 和 Dock 不是两种不同的拖动模式，而是同一次 Panel Drag 的两个不同 Drop Result。**
