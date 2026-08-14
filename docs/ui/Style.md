# 任务：将现有 DX12 UI 框架升级为 UE Editor 风格

当前项目是一个基于 C++ / DX12 的自研 UI 框架。

Dock、Panel、PanelGroup 等核心交互已经存在，但当前 UI：

* 视觉过于简陋；
* 控件缺乏统一风格；
* Panel / PanelGroup 层级感不足；
* Tab、Button、Toolbar、Tree 等交互状态不够明确；
* 缺少统一的图标系统；
* 整体不像成熟的 Editor / Tool UI。

本任务需要在**不重新设计现有 UI 架构**的前提下，将视觉语言升级为接近 Unreal Engine 5 Editor 的现代暗色工具 UI。

目标是：

> 参考 UE Editor 的视觉密度、层次、控件状态、图标语言和编辑器气质，实现属于当前项目自己的 Editor Theme。

不要直接复制 Unreal Engine 的图片、SVG、字体、纹理或其他专有资源。

可以参考其：

* 视觉语言；
* 控件结构；
* 图标语义；
* 尺寸比例；
* 层次关系；
* Hover / Active / Selected 等状态表现。

图标应使用项目自己的资源或允许使用的开源资源重新实现。

---

# 1. 工作方式

使用单 Agent。

不要启动 Subagent。

首先阅读现有：

* UI Theme / Style；
* Painter；
* DX12 UI Renderer；
* Button；
* Text；
* Panel；
* PanelGroup；
* Tab；
* Menu；
* Toolbar；
* Tree；
* Scrollbar；
* Input / Hover / Focus 状态；
* Icon / Texture 相关代码。

然后输出最多 6 条实施计划。

不要提出多个视觉架构方案。

不要重新设计：

* DockTree；
* PanelGroup；
* Layout；
* Input；
* Renderer；
* Drag & Drop。

本任务主要是：

```text
Style
+
Painter
+
Widget visual state
+
Icon rendering
```

的增量升级。

---

# 2. 最重要的原则：Theme 驱动

禁止继续在控件内部散落：

```cpp
Color(0.13f, 0.13f, 0.13f)
Color(0.18f, 0.18f, 0.18f)

padding = 6
height = 24
radius = 3
```

建立或完善一个统一：

```cpp
UITheme
```

或者复用当前已有 Style 系统。

所有编辑器控件的：

* Color；
* Spacing；
* Padding；
* Radius；
* Border；
* Text Color；
* Icon Color；
* Control Height；
* Tab Height；
* TitleBar Height；

尽可能从 Theme 获取。

不要创建复杂的 CSS 系统。

不要创建运行时 Style DSL。

只需要一个简单、明确、低开销的 C++ Theme。

---

# 3. Theme 建议结构

优先适配现有代码。

概念上可以类似：

```cpp
struct UITheme
{
    struct Colors
    {
        Color windowBackground;

        Color panelBackground;
        Color panelBackgroundRaised;
        Color panelBackgroundSunken;

        Color titleBar;
        Color titleBarActive;

        Color controlNormal;
        Color controlHovered;
        Color controlPressed;
        Color controlSelected;

        Color border;
        Color borderSubtle;
        Color separator;

        Color textPrimary;
        Color textSecondary;
        Color textDisabled;

        Color iconPrimary;
        Color iconSecondary;
        Color iconDisabled;

        Color accent;
        Color accentHovered;
        Color accentPressed;

        Color selection;
        Color focus;

        Color danger;
    };

    struct Metrics
    {
        float titleBarHeight;
        float tabHeight;
        float toolbarHeight;
        float menuItemHeight;
        float treeRowHeight;
        float controlHeight;

        float paddingXS;
        float paddingS;
        float paddingM;
        float paddingL;

        float iconSmall;
        float iconNormal;
        float iconLarge;

        float borderWidth;
        float cornerRadius;
    };
};
```

这是示意，不要求完全照搬。

如果当前项目已经存在 Style 数据结构，应优先扩展，而不是创建第二套系统。

---

# 4. 整体视觉方向

视觉目标：

```text
Dark
Compact
Professional
Editor-oriented
Low visual noise
Clear hierarchy
Strong interaction feedback
```

不要做：

```text
大圆角
大阴影
卡片化 SaaS UI
高饱和渐变
大量透明玻璃
超大 Padding
移动端风格按钮
```

这是桌面 Editor UI。

更接近：

```text
Unreal Editor
Visual Studio
Rider
Blender
专业 DCC / IDE
```

但视觉基准优先 UE Editor。

---

# 5. 推荐基础颜色关系

不要把下面数值视为 Unreal Engine 的原始颜色。

它们只是当前项目的初始设计建议。

保持整体中性、低饱和。

可以从类似以下层级开始：

```text
Workspace Background
≈ very dark neutral

Panel Background
≈ slightly lighter

Raised Surface
≈ controls / toolbar

Hovered Surface
≈ clearly visible but subtle

Pressed / Selected
≈ stronger contrast

Border
≈ low contrast

Primary Text
≈ soft white

Secondary Text
≈ medium gray

Disabled
≈ dark gray

Accent
≈ restrained blue
```

参考起点：

```cpp
windowBackground      ≈ #171719

panelBackground       ≈ #202022
panelBackgroundRaised ≈ #262629
panelBackgroundSunken ≈ #19191B

titleBar              ≈ #252528
titleBarActive        ≈ #29292C

controlNormal         ≈ #29292C
controlHovered        ≈ #343438
controlPressed        ≈ #1E1E21
controlSelected       ≈ #38383D

border                ≈ #3A3A3D
borderSubtle          ≈ #2D2D30
separator             ≈ #363639

textPrimary           ≈ #D6D6D6
textSecondary         ≈ #A0A0A5
textDisabled          ≈ #66666B

iconPrimary           ≈ #C8C8CD
iconSecondary         ≈ #96969C
iconDisabled          ≈ #606066

accent                ≈ #3578B8
accentHovered         ≈ #438ACB
accentPressed         ≈ #28689F
```

不要机械照抄这些颜色。

最终应通过项目实际截图进行调整。

重点是建立：

```text
background hierarchy
interaction hierarchy
text hierarchy
```

而不是追求某几个 RGB 数值。

---

# 6. UI 层级

编辑器 UI 必须明确区分不同 Surface。

建议：

```text
Level 0
Workspace background

Level 1
Docked PanelGroup

Level 2
TitleBar / Toolbar

Level 3
Button / Input / ComboBox

Level 4
Popup / Menu / Tooltip
```

不要所有区域都使用完全相同的背景色。

需要通过非常细微的亮度变化表现层级。

不要依赖厚边框。

优先：

```text
surface brightness
+
1px separator
+
state contrast
```

形成层级。

---

# 7. PanelGroup

PanelGroup 是最重要的视觉组件之一。

目标类似专业 Editor Dock Window：

```text
┌─────────────────────────────────────────┐
│ Scene │ Game │ Inspector           ×   │
├─────────────────────────────────────────┤
│                                         │
│                                         │
│             Panel Content               │
│                                         │
│                                         │
└─────────────────────────────────────────┘
```

PanelGroup 应具有：

* 独立 Header；
* Tab Strip；
* Close Button；
* 明确的 Active Tab；
* Hover Tab；
* Inactive Tab；
* Content separator；
* Docked / Floating 一致的视觉语言。

不要给每个 Panel 加大边框。

---

# 8. Tab 风格

Tab 应高度紧凑。

推荐：

```text
高度约 26–30 logical px
```

实际数值根据当前 DPI / UI scale 体系决定。

Inactive：

```text
背景接近 Header
文字 Secondary
```

Hovered：

```text
背景略微提亮
文字 Primary
```

Active：

```text
背景与 Content Surface 建立连续关系
文字 Primary
可以增加非常轻微的 Accent indicator
```

例如：

```text
          Active
            ↓

 Scene │ Game │ Inspector
───────────────
```

或者使用：

```text
2px accent underline
```

不要：

* 大面积蓝色 Active Tab；
* 强渐变；
* 很大的圆角；
* Active Tab 浮成一张卡片。

---

# 9. Tab Close / Group Close

关闭图标使用：

```text
×
```

或自己的 Close SVG/Icon。

正常状态：

```text
低对比
```

Hover：

```text
提高亮度
```

Pressed：

```text
背景略暗
```

危险操作可以在 Hover 时略微引入 danger 色，但不要默认红色。

Close Button 不应该有永久明显背景。

---

# 10. Toolbar

Toolbar 应采用 UE Editor 风格的紧凑布局：

```text
┌─────────────────────────────────────────────────┐
│ ▶  ■  │ Save │ Build ▼ │          Settings ⚙   │
└─────────────────────────────────────────────────┘
```

强调：

* 图标；
* 图标 + 文本；
* 小间距；
* Group separator；
* subtle hover；
* 几乎没有默认按钮边框。

默认 Toolbar Button：

```text
transparent / surface background
```

Hovered：

```text
subtle raised background
```

Pressed：

```text
dark / selected background
```

不要让 Toolbar 看起来像一排普通 PushButton。

---

# 11. Button

至少区分：

```text
Normal
Hovered
Pressed
Disabled
Selected / Toggle
```

状态变化应主要通过：

```text
Background
Text
Icon tint
```

表达。

不要依赖缩放动画。

Editor UI 应保持稳定。

---

# 12. Icon System

建立一个轻量、统一的 Icon System。

不要在业务代码中直接：

```cpp
DrawImage(texture123, ...)
```

应能够表达：

```cpp
DrawIcon(UIIcon::Close, ...);
DrawIcon(UIIcon::Play, ...);
DrawIcon(UIIcon::Folder, ...);
DrawIcon(UIIcon::Search, ...);
```

例如：

```cpp
enum class UIIcon
{
    None,

    Close,

    ChevronLeft,
    ChevronRight,
    ChevronUp,
    ChevronDown,

    Add,
    Remove,

    Search,
    Settings,

    Folder,
    FolderOpen,
    File,

    Save,

    Play,
    Pause,
    Stop,

    Refresh,

    Visibility,
    Hidden,

    Lock,
    Unlock,

    Menu,
    More,

    Check,

    Warning,
    Error,
    Info
};
```

只加入当前实际 UI 使用的 Icon。

不要为了“完整 Icon Framework”一次加入几百个 enum。

---

# 13. 图标视觉语言

图标整体参考 UE Editor 中工具型图标的感觉：

```text
simple
compact
high readability
consistent stroke/weight
recognizable at 14–18px
low visual noise
```

优先采用：

```text
Monochrome
+
Tint
```

而不是每个 Icon 一张彩色 PNG。

典型尺寸：

```text
Small
12–14 px

Normal
16 px

Toolbar
16–20 px
```

具体根据当前 DPI 系统调整。

图标必须在高 DPI 下保持清晰。

---

# 14. 图标资源

不要复制 Unreal Engine 自带 Icon Asset。

可以：

1. 使用项目自己设计的 SVG；
2. 使用许可证允许的开源 Icon Set；
3. 使用简单 Path/Icon Geometry；
4. 将 SVG 转为项目自己的 Texture Atlas。

如果当前 Renderer 已经支持 Texture Atlas：

优先扩展现有 Atlas。

不要为了图标系统重做 Renderer。

如果当前只支持 texture：

第一版可以使用：

```text
single icon atlas texture
+
UV Rect
+
tint
```

即可。

---

# 15. Icon Registry

可以使用非常轻量的映射：

```cpp
struct IconInfo
{
    TextureHandle texture;

    Rect uv;

    Vec2 nominalSize;
};
```

例如：

```cpp
const IconInfo& GetIcon(UIIcon icon);
```

或者如果全放在 Atlas：

```cpp
struct IconAtlasEntry
{
    Rect uv;
    Vec2 size;
};
```

不要引入：

```text
IconManager
IconProvider
IconFactory
IconResolver
IconThemeStrategy
```

等多层抽象。

除非现有架构明确要求。

---

# 16. Tree / Hierarchy

Hierarchy Tree 是 Editor 气质非常重要的一部分。

参考：

```text
▾ World
    ◇ Camera
    ▸ Character
    ▸ Environment
```

应支持明确的：

```text
Row Normal
Row Hover
Row Selected
Row Focused

Expand Arrow
Icon
Label
Optional secondary indicator
```

Tree Row 应：

* 紧凑；
* 高度一致；
* selection 覆盖整行；
* indentation 清晰；
* icon 与 text 对齐。

不要给每行画矩形边框。

---

# 17. Property / Inspector 风格

如果项目已有属性编辑器，应采用：

```text
Property Name          Value
─────────────────────────────
Transform
  Position X        [ 0.0 ]
           Y        [ 1.0 ]
           Z        [ 0.0 ]
```

重点：

* Label / Value 对齐；
* compact row；
* group hierarchy；
* subtle separator；
* restrained input background。

不要把每个属性包装成 Card。

---

# 18. Input / TextBox

TextBox：

Normal：

```text
dark sunken background
subtle border
```

Hover：

```text
slightly brighter border
```

Focused：

```text
accent border
```

Disabled：

```text
lower contrast
```

Focused 状态必须明显，但不要使用强烈 glow。

---

# 19. ComboBox

ComboBox 风格：

```text
┌───────────────────┬───┐
│ Perspective       │ ▼ │
└───────────────────┴───┘
```

箭头使用统一：

```cpp
UIIcon::ChevronDown
```

Popup Menu 必须和其他 Editor Popup 使用相同 Surface Theme。

---

# 20. Scrollbar

Scrollbar 不应成为视觉焦点。

正常：

```text
低对比 Track
细 Thumb
```

Hover：

```text
Thumb 提亮
```

Dragging：

```text
Thumb 更明显
```

尽量紧凑。

不要使用系统默认粗 Scrollbar 外观。

---

# 21. Splitter

Dock Splitter：

Normal：

```text
几乎不可见
```

Hover：

```text
明显一点
```

Dragging：

```text
accent / highlighted
```

视觉宽度和 HitTest 宽度可以不同：

```text
visual width ≈ 1px
hit width ≈ 4–6px
```

如果当前架构允许。

不要为了增加 hit width 改变 Dock layout。

---

# 22. Menu

Editor Menu：

```text
File
Edit
Window
Help
```

Popup：

```text
┌──────────────────────────────┐
│ New                    Ctrl+N │
│ Open...                Ctrl+O │
│ Save                   Ctrl+S │
├──────────────────────────────┤
│ Recent                    ›  │
└──────────────────────────────┘
```

Menu Item 应支持：

```text
Icon
Label
Shortcut
Submenu arrow
Check state
Disabled state
```

但只实现项目当前需要的部分。

不要顺便建立完整 Command Framework。

---

# 23. Tooltip

Tooltip 使用：

```text
dark raised surface
small text
small padding
subtle border
```

不要：

* 大阴影；
* 大圆角；
* 长动画。

---

# 24. Focus / Selection

必须统一：

```text
Hover
Pressed
Selected
Focused
Active
```

不要混为一谈。

例如 Tree：

```text
Hovered
≠
Selected
```

Tab：

```text
Hovered
≠
Active
```

TextBox：

```text
Hovered
≠
Focused
```

所有 Widget 应使用一致的 state → style 映射方式。

---

# 25. Typography

Editor UI 使用紧凑、清晰的无衬线字体。

优先使用项目已有字体。

不要因为模仿 UE Editor 而导入 Unreal Engine 字体文件。

建立：

```text
Default
Small
Heading
Muted
```

少量层级即可。

不要创建十几种字号。

建议整体：

```text
Default ≈ 13–14 logical px
Small   ≈ 11–12
Heading ≈ 14–16
```

具体根据现有字体实际视觉效果调整。

---

# 26. Spacing

Editor UI 应明显比普通 Web UI 更紧凑。

建立统一 spacing scale：

```text
XS
S
M
L
```

例如概念上：

```text
2
4
6
8
```

或者：

```text
2
4
8
12
```

根据当前 UI scale 调整。

不要在不同 Widget 中随意出现：

```text
5
7
11
13
```

这种随机 Magic Number。

---

# 27. Corner Radius

整体圆角应非常克制。

Panel：

```text
0 或接近 0
```

Button / Input：

```text
小圆角
```

Popup：

```text
非常轻微圆角
```

不要将 Editor 做成现代 Web Dashboard 的大圆角卡片风格。

---

# 28. Border

避免所有东西都有 Border。

优先：

```text
surface contrast
```

Border 主要用于：

* Focus；
* Input；
* Popup；
* Separator；
* 特殊强调。

大量 1px 边框会让界面过于嘈杂。

---

# 29. Hover

Hover 是整个视觉升级的关键。

所有可交互区域必须有 Hover feedback：

```text
Tab
Button
Toolbar Button
Tree Row
Menu Item
Close
Icon Button
ComboBox
Scrollbar
Splitter
```

但 Hover 应 subtle。

不要出现：

```text
Normal = dark gray
Hover = bright blue
```

Accent 主要保留给：

```text
Selected
Focused
Important action
```

---

# 30. IconButton

建立或复用一个简单的 Icon Button 表现方式：

```text
┌──────┐
│  ⚙   │
└──────┘
```

但 Normal 状态通常不需要明显背景。

主要状态：

```text
Normal
Hover
Pressed
Disabled
Selected
```

用于：

* Close；
* Settings；
* Visibility；
* Refresh；
* More；
* Toolbar actions。

如果当前 Button 已经可以支持 Icon，不要新建完整 Widget 类型。

---

# 31. DX12 Renderer 边界

不要让 DX12 Renderer 知道：

```text
Button
PanelGroup
Tab
Toolbar
Tree
Theme
```

Renderer 仍然只处理类似：

```text
Rect
Border
Text
Image/Icon
Clip
DrawCommand
```

如果当前 Painter 缺少：

```cpp
DrawIcon(...)
```

应该在 UI Painter / UI drawing abstraction 中实现，然后转换为普通 image draw command。

不要把 UIIcon enum 传入 DX12 backend。

---

# 32. 性能要求

这是 Editor UI，所有 Widget 每帧都会大量执行。

避免：

* 每帧创建 Texture；
* 每帧解析 SVG；
* 每帧分配 Style；
* 每帧创建 string-based style lookup；
* 每个 Icon 单独 texture allocation；
* 为简单 state change 创建复杂动画对象。

Theme 和 Icon metadata 应：

```text
stable
cheap
cache-friendly
```

Icon Atlas 应加载一次。

---

# 33. 第一阶段优先级

不要尝试一次重做所有 Widget。

优先级：

## P0

先实现：

```text
UITheme
Icon rendering
PanelGroup
Tabs
TitleBar
Close Button
Toolbar
Button
```

这些决定整体第一视觉印象。

---

## P1

然后：

```text
Tree
Scrollbar
Splitter
Menu
ComboBox
TextBox
```

---

## P2

最后根据项目实际存在的控件继续统一：

```text
Inspector
Property Row
Slider
Checkbox
Context Menu
Tooltip
Status Bar
```

不要实现项目不存在的 Widget。

---

# 34. PanelGroup 最终目标

PanelGroup 应接近：

```text
┌─────────────────────────────────────────────────────┐
│ Scene │ Game │ Inspector                     ×      │
├─────────────────────────────────────────────────────┤
│                                                     │
│                                                     │
│                    Content                          │
│                                                     │
│                                                     │
└─────────────────────────────────────────────────────┘
```

Active Tab：

```text
Primary text
Clear active surface
Optional subtle accent
```

Inactive：

```text
Secondary text
Header surface
```

Hover：

```text
Raised slightly
```

Close：

```text
subtle until hover
```

PanelGroup Floating 与 Docked 应保持相同基本视觉语言。

---

# 35. Interaction 不允许回归

视觉修改不能破坏已有：

```text
Panel Drag
PanelGroup Drag

Panel → Center merge

PanelGroup → Center Floating

Edge Dock

Floating

Close

Tab switching

Splitter resize
```

视觉 Hit Region 和真实 HitTest Region 必须保持一致。

特别注意新的：

```text
padding
icon
tab geometry
title bar height
```

不能导致 Drag HitTest 与渲染位置错位。

---

# 36. Visual Debug

如果项目已有 Debug Overlay，增加轻量能力显示：

```text
Widget bounds
Hovered widget
Focused widget
Pressed widget
PanelGroup header rect
Tab rect
Content rect
```

仅在已有 debug infrastructure 容易扩展时实现。

不要为了本任务创建大型 UI Inspector。

---

# 37. Reference Comparison

如果仓库存在：

```text
references/
```

或者任务提供 UE Editor 截图：

优先观察：

```text
1. Surface hierarchy
2. Header height
3. Tab height
4. Toolbar density
5. Icon size
6. Text/icon alignment
7. Padding
8. Hover contrast
9. Selected contrast
10. Separator strength
```

不要只比较颜色。

视觉相似度更多来自：

```text
proportion
density
spacing
hierarchy
state feedback
```

而不是 RGB 完全一致。

---

# 38. 不要复制 Unreal Engine 资源

重要：

不要：

* 从 Unreal Engine 安装目录复制 Icons；
* 复制 Unreal Engine SVG；
* 复制 Unreal Engine PNG；
* 复制 Unreal Engine Fonts；
* 将 Unreal Engine Editor assets 加入项目。

目标是：

```text
UE-inspired editor visual language
```

而不是：

```text
UE asset extraction
```

使用：

```text
own assets
or
properly licensed open-source assets
```

实现类似语义。

---

# 39. Complexity Budget

本任务是视觉升级，不是架构重写。

禁止因为 Style 升级新增大量：

```text
Manager
Factory
Provider
Strategy
Resolver
Theme graph
Style inheritance system
CSS engine
runtime reflection
```

优先：

```text
existing Theme extension
+
small Icon Registry
+
Widget paint adjustment
```

如果已有：

```cpp
ButtonStyle
TabStyle
TextStyle
```

则复用。

如果没有，不要马上设计完整 Style hierarchy。

---

# 40. 修改原则

同样正确时优先选择：

```text
更少的新类型
更少的 runtime state
更少的 allocation
更少的间接调用
更少的修改文件
更简单的 drawing path
```

但不要重新散布 Magic Number。

视觉常量应该集中进入 Theme。

---

# 41. 实施顺序

严格按以下顺序工作：

```text
1. Audit existing style code

2. Define theme tokens

3. Apply global background/text hierarchy

4. Implement/refine icon rendering

5. Upgrade PanelGroup + Tab + TitleBar

6. Upgrade Toolbar + Buttons

7. Upgrade Tree / Input / Scrollbar / Menu

8. Compile

9. Run UI demo/editor

10. Fix visual state and interaction regressions
```

不要先重构整个 UI Framework。

---

# 42. 验收要求

最终至少满足：

### Global

* 整个 Editor 采用统一暗色 Theme；
* 不再大量散落颜色 Magic Number；
* Surface 层级能够明显区分；
* Text 层级统一；
* Spacing 统一。

### PanelGroup

* Header 视觉接近成熟 Editor；
* Tab 紧凑；
* Active / Hover / Inactive 明确；
* Close Button 使用统一 Icon；
* Floating 和 Docked 风格一致。

### Icons

* 建立统一 Icon 调用方式；
* Icon 大小统一；
* Icon tint 与 Widget State 联动；
* 不使用 UE 原始资源。

### Toolbar

* 紧凑；
* 图标优先；
* Hover 明确；
* Separator 清晰但不抢眼。

### Controls

至少 Button / Tree / TextBox / ComboBox / Scrollbar 中项目已经存在的控件具有一致：

```text
Normal
Hover
Pressed
Selected
Disabled
Focused
```

视觉语言。

### Architecture

* Dock 架构不改变；
* Panel / PanelGroup drag 行为不改变；
* Layout ownership 不改变；
* Renderer 不承担 Widget Style 逻辑。

---

# 43. 最终检查

完成后主动搜索本次涉及 Widget 中仍然存在的：

```cpp
hard-coded colors
hard-coded icon sizes
hard-coded control heights
duplicated hover colors
duplicated border colors
```

将真正属于 Theme 的值集中。

但不要顺便清理与本任务无关的旧代码。

---

# 44. 最终汇报

最终只报告：

1. Theme 增加了哪些主要 token；
2. 哪些 Widget 完成视觉升级；
3. Icon System 如何实现；
4. 使用了哪些 Icon 资源；
5. 是否存在仍未统一的 Widget；
6. 构建 / 测试结果；
7. 是否发现 UI HitTest 或交互回归。

不要输出新的大型 UI 架构方案。
