# UI 控件属性参考

本文档列出 `ControlFactory` 当前注册的全部声明式控件，以及它们通过
`IProperty::SetProperty(name, value)` 实际接受的属性。XAML、未来属性检查器和
直接调用 `SetProperty` 的代码共用这套属性协议。

文档只把能通过 `SetProperty` 设置的字段称为“属性”。例如 `Visible`、
`ClipChildren`、`FontFamily`、`Terminal::MaxLines` 虽然是公开 C++ 状态，但目前
没有声明属性入口，因此不在控件的合法 XAML 属性集合中。

## 取值格式与解析规则

属性名、控件名和枚举值区分大小写。未知属性会使 XAML 加载失败，并产生英文
错误 `Control <type> does not support attribute <name>`。

| 类型 | 合法写法 | 说明 |
| --- | --- | --- |
| 布尔值 | `true`、`True`、`1`；`false`、`False`、`0` | 不接受其他大小写或 `yes/no` |
| 浮点数 | 例如 `0`、`12.5`、`-3`、`1e2` | 应提供完整、有限的 `float` 字面量 |
| 颜色 | `#RRGGBB`、`#RRGGBBAA` | 两位十六进制通道，省略 Alpha 时为 `FF` |
| 颜色 | `r,g,b` 或 `r,g,b,a` | 每个通道必须在 `[0, 1]` 内 |
| 字符串 | 任意 XML 属性字符串 | XML 实体会先解码，例如 `&amp;` 变为 `&` |

除布尔、颜色、枚举以及另行注明的范围检查外，现有数值解析大多直接使用
`std::strtof`。部分入口尚未检查尾随字符或非有限值；文档把“完整、有限的数字
字面量”定义为合法输入，不应依赖非法文本当前可能退化为 `0` 的实现细节。

## 公共布局属性（L）

所有已注册控件都继承 `BaseNode`，因此都接受以下属性。

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Id` / `Key` / `Name` | 三个别名，设置节点的稳定 `Key` | 字符串；需要查找或热重载复用时应非空；同一 XAML 树中的非空 Key 必须唯一 |
| `Width` | 首选宽度 | 浮点数；当前公共解析器不限制正负 |
| `Height` | 首选高度 | 浮点数；当前公共解析器不限制正负 |
| `MinWidth` | 最小宽度 | 浮点数；带 Resize Behavior 的控件会优先按 Resize 规则处理 |
| `MinHeight` | 最小高度 | 浮点数；带 Resize Behavior 的控件会优先按 Resize 规则处理 |
| `MaxWidth` | 最大宽度 | 浮点数；应不小于 `MinWidth` |
| `MaxHeight` | 最大高度 | 浮点数；应不小于 `MinHeight` |
| `FlexGrow` | 父容器主轴有剩余空间时的增长权重 | 浮点数；通常使用 `>= 0` |
| `FlexShrink` | 主轴空间不足时的收缩权重 | 浮点数；通常使用 `>= 0` |
| `Margin` | 节点外侧四边统一间距 | 浮点数；当前只支持四边同值 |
| `Padding` | 内容内侧四边统一间距 | 浮点数；当前只支持四边同值 |
| `Direction` | 子节点的 Flex 主轴方向 | `Row` 或 `Column` |

当前 XAML 属性协议没有暴露 `Position`、`Left`、`Top`、`Right`、`Bottom`、
`WidthPercent`、分边 Margin/Padding 或 `auto`。这些字段只能由控件内部或 C++
直接写入 `LayoutStyle`。

## 公共绘制属性（V）

继承 `DrawNode` 的控件接受以下属性。纯布局控件 `UI`、`Scroll`、`TreeView` 和
`Scene` 没有自己的矩形渲染对象，因此不接受这些属性。

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Color` | 控件根矩形的填充色 | 合法颜色 |
| `Border` / `BorderWidth` | 边框像素宽度 | 浮点数且 `>= 0` |
| `BorderColor` | 边框颜色 | 合法颜色 |
| `CornerRadius` / `Radius` | 屏幕空间圆角半径 | 浮点数且 `>= 0`；Shader 会按最终矩形尺寸夹紧 |

复合控件的 `Color` 只覆盖根矩形，不自动改写内部标题、文字、轨道或 Popup。
控件状态变化也可能重新应用 Theme 语义色；需要稳定定制完整复合控件时应扩展
对应 Theme，而不是假设一个根 `Color` 能覆盖所有内部视觉。

## Behavior 属性组

Behavior 属性只有在控件构造时确实挂载了对应 Behavior 时才合法。后文的控件
矩阵给出每个标签拥有的属性组。

### Drag 属性（D）

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Draggable` / `DragEnabled` | 是否允许 Drag Behavior 启动手势 | 布尔值 |
| `DragRegion` | 可以开始拖动的区域 | `TitleBar` 或 `Anywhere` |

`TitleBar` 需要控件为 Drag Behavior 绑定标题句柄。`Panel` 和 `Scene` 已绑定；
普通控件即使由 C++ 动态添加 Drag Behavior，没有句柄时也应使用 `Anywhere`。

### Resize 属性（R）

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Resizable` / `ResizeEnabled` | 是否允许从边缘或四角拉伸 | 布尔值 |
| `ResizeBorder` | 边缘命中区域的逻辑像素宽度 | 解析后夹紧到 `>= 0` |
| `MinWidth` | Resize 手势允许的最小宽度，同时同步到布局最小值 | 解析后夹紧到 `>= 1` |
| `MinHeight` | Resize 手势允许的最小高度，同时同步到布局最小值 | 解析后夹紧到 `>= 1` |

`MinWidth/MinHeight` 与公共布局属性同名。Behavior 的解析优先级高于
`BaseNode`，所以 `Panel`、`PanelGroup` 和 `Scene` 上设置它们时采用本表的
`>= 1` 规则。

### Scroll 属性（S）

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Scrollable` / `ScrollEnabled` | Scroll Behavior 总开关 | 布尔值 |
| `HorizontalScrollEnabled` | 是否允许水平方向滚动 | 布尔值 |
| `VerticalScrollEnabled` | 是否允许垂直方向滚动 | 布尔值 |
| `HorizontalScrollBar` | 水平滚动条显示策略 | `Hidden`、`Auto` 或 `Visible` |
| `VerticalScrollBar` | 垂直滚动条显示策略 | `Hidden`、`Auto` 或 `Visible` |
| `ShowHorizontalScrollBar` | 水平滚动条的布尔便捷属性 | 布尔值；`true` 映射 `Visible`，`false` 映射 `Hidden` |
| `ShowVerticalScrollBar` | 垂直滚动条的布尔便捷属性 | 布尔值；`true` 映射 `Visible`，`false` 映射 `Hidden` |
| `WheelStep` | 每个滚轮刻度的垂直滚动距离 | 解析后夹紧到 `>= 1` |

当前 `ScrollNode` 只绑定了垂直滚动范围和垂直滚动条。水平属性可以被解析和
保存，但尚没有水平位移/水平滚动条实现，不能把它们视为完整的水平滚动支持。

### Dock 属性（K）

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `DockEnabled` | 是否参与 DockWorkspace 初始布局和 Dock 手势 | 布尔值 |
| `Dock` | 初次建立 DockTree 时的放置策略 | `Auto`、`Left`、`Right`、`Top`、`Bottom` 或 `Fill` |
| `DockThreshold` | 拖动时进入边缘候选区的逻辑像素阈值 | 解析后夹紧到 `>= 0` |
| `DockExtent` | 初次边缘停靠时沿切割轴占用的逻辑像素 | 解析后夹紧到 `>= 1` |

`Dock` 只描述初始结构，运行时最终几何由 DockTree 管理。`DockProperty` 内部还
有 `Resizable` 字段，但目前没有对应 XAML/`SetProperty` 属性；ToolBar 在构造
时把它设为 `false`，因此不会生成可拉伸的相邻 Splitter。

## 控件属性总览

下表中的 L/V/D/R/S/K 分别引用前面的公共布局、公共绘制、Drag、Resize、
Scroll 和 Dock 属性组。“专属属性”列列出该控件额外接受的全部名称。

| XAML 标签 | 属性组 | 专属属性 | 备注 |
| --- | --- | --- | --- |
| `UI` | L | 无 | 根/结构容器，没有矩形视觉 |
| `Rect` | L + V | 无 | 通用可绘制矩形 |
| `Text` | L | `Text`、`FontSize`、`TextColor`、`TextAlignment`、`TextWrap` | 由 DirectWrite 通道绘制，不接受 V |
| `Image` | L + V | `Source`、`Tint`、`TintColor` | `Tint/TintColor` 是 `Color` 的别名 |
| `Button` | L + V | `Text`、`Label`、`Enabled` | `Text/Label` 为别名 |
| `Toggle` | L + V | `Text`、`Label`、`Checked`、`Value`、`Enabled` | `Checked/Value` 为别名 |
| `Slider` / `Slide` | L + V | `Minimum`、`Min`、`Maximum`、`Max`、`Value`、`Step`、`Enabled` | `Slide` 仅是标签别名，运行时仍为 Slider |
| `TextInput` | L + V | `Text`、`Value`、`Placeholder`、`Hint`、`Enabled` | `Text/Value`、`Placeholder/Hint` 分别为别名 |
| `TreeView` | L + S | 无 | 复用无根矩形的 ScrollNode |
| `FileExplorer` | L + S | `RootPath`/`Path`、`ShowHidden` | 基于 TreeView 的只读文件目录浏览器 |
| `TreeItem` | L + V | `Text`、`Label`、`Expanded`、`Selected`、`Enabled` | 只能作为 TreeView/TreeItem 的层级项使用 |
| `Menu` | L + V | `Text`、`Label`、`Open`、`Expanded`、`Enabled` | `Open/Expanded` 为别名 |
| `MenuItem` | L + V | `Text`、`Label`、`Enabled` | 叶子命令项 |
| `ToolBar` / `Toolbar` | L + V + K | 无 | `Toolbar` 是标签别名；固定 Top/不可分割是构造默认值 |
| `Scroll` | L + S | 无 | 无根矩形，声明子节点进入内部 Content |
| `Panel` | L + V + D + R + S + K | `Icon`、`TitleIcon`、`Title`、`TitleHeight`、`TitleColor`、`TitleTextColor` | `Icon/TitleIcon` 为别名 |
| `PanelGroup` | L + V + R + K | 无 | 标题来自子 Panel；空白标题区不产生 Group Dock 手势 |
| `Scene` | L + D + R + K | `Title`、`TitleHeight` | 无根矩形；3D 内容只绘制到内部 viewport |
| `Terminal` | L + V + D + R + S + K | 与 Panel 相同 | Panel 的专用输出派生类 |

## 控件专属属性

### `UI`

只接受 L。它是结构和布局根，不接受 `Color`、Behavior 属性或可见性属性。

### `Rect`

只接受 L + V，没有专属属性。

### `Text`

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Text` | UTF-8 显示文本 | 字符串 |
| `FontSize` | DirectWrite 字号，并把最小行高更新为字号的 1.25 倍 | 解析后夹紧到 `>= 1` |
| `TextColor` | 字体颜色 | 合法颜色 |
| `TextAlignment` | 文本框内的水平对齐 | `Leading`、`Center` 或 `Trailing` |
| `TextWrap` | 是否允许换行 | 布尔值 |

`FontFamily` 当前由 C++/Theme 提供，默认值为 `Microsoft YaHei`（微软雅黑），
没有声明属性。`Text` 也不接受 `Color`，应使用 `TextColor`。

### `Image`

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Source` | 选择内建单色图标 | 仅接受下方注册 URI |
| `Tint` / `TintColor` | 图标着色 | 合法颜色；等价于根 `Color` |

规范资源 URI：

- `asset://texture/icons/lucide/x.svg`
- `asset://texture/icons/lucide/plus.svg`
- `asset://texture/icons/lucide/chevron-down.svg`
- `asset://texture/icons/lucide/box.svg`
- `asset://texture/icons/lucide/terminal.svg`
- `asset://texture/icons/lucide/settings-2.svg`
- `asset://texture/icons/lucide/chevron-right.svg`

兼容 URI：

- `builtin://icon/close`
- `builtin://icon/plus`
- `builtin://icon/chevron-down`
- `builtin://icon/cube`
- `builtin://icon/chevron-right`

其他路径当前返回“不支持属性值”，并不会自动加载任意图片文件。

### `Button`

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Text` / `Label` | 按钮标签 | 字符串 |
| `Enabled` | 是否接受鼠标和键盘激活 | 布尔值 |

### `Toggle`

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Text` / `Label` | 开关标签 | 字符串 |
| `Checked` / `Value` | 当前布尔选择状态 | 布尔值 |
| `Enabled` | 是否允许改变状态 | 布尔值 |

### `Slider` / `Slide`

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Minimum` / `Min` | 数值范围下限 | 有限浮点数；若不小于当前 Maximum，会临时把 Maximum 调为 `Minimum + 1` |
| `Maximum` / `Max` | 数值范围上限 | 有限浮点数；若不大于当前 Minimum，会临时把 Minimum 调为 `Maximum - 1` |
| `Value` | 当前值 | 浮点数；夹紧到 `[Minimum, Maximum]`，并按 Step 量化 |
| `Step` | 步进间隔 | 浮点数且 `>= 0`；`0` 表示连续值 |
| `Enabled` | 是否接受鼠标和键盘调整 | 布尔值 |

Min 与 Max 的设置顺序不影响最终范围有效性。为避免 `Value` 在范围尚未建立时
被默认范围提前夹紧，XAML 中应先写 `Min/Max`，再写 `Value`。

### `TextInput`

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Text` / `Value` | 当前单行 UTF-8 文本 | 字符串 |
| `Placeholder` / `Hint` | Text 为空时显示的提示文字 | 字符串 |
| `Enabled` | 是否允许聚焦和编辑 | 布尔值 |

当前属性面不包含只读、密码模式、最大长度、选择范围、IME 模式或多行模式。

### `TreeView`

只接受 L + S。声明的 `TreeItem` 会进入内部滚动 Content；TreeView 自己管理
唯一选择项，但没有 `SelectedItem` 字符串属性。

### `FileExplorer`

继承 TreeView 的 L + S 属性，并由控件扫描目录生成 TreeItem；调用方仍负责文件
打开、导入等业务行为。目录排在文件之前，同类按文件名排序，扫描不会跟随目录
符号链接。

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `RootPath` / `Path` | 显示的根目录 | UTF-8 文件系统路径；不存在时显示禁用的 `(not found)` 根项 |
| `ShowHidden` | 是否显示名称以 `.` 开头的项 | 布尔值；默认 `false` |

### `TreeItem`

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Text` / `Label` | 行标题 | 字符串 |
| `Expanded` | 是否显示递归子项 | 布尔值 |
| `Selected` | 是否声明为当前选择项 | 布尔值；TreeView 布局后只保留一个选择项 |
| `Enabled` | 是否允许选择和展开交互 | 布尔值 |

### `Menu`

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Text` / `Label` | 顶层入口或级联目录标题 | 字符串 |
| `Open` / `Expanded` | Popup 是否展开 | 布尔值 |
| `Enabled` | 是否允许打开和导航 | 布尔值 |

顶层与级联样式由它是否位于 ToolBar/另一个 Menu 中自动决定，没有
`TopLevel` 声明属性。

### `MenuItem`

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Text` / `Label` | 命令标题 | 字符串 |
| `Enabled` | 是否允许激活 | 布尔值 |

快捷键、图标、复选状态和 XAML 命令绑定目前没有属性入口。

### `ToolBar` / `Toolbar`

只接受 L + V + K，没有专属属性。构造时默认 `Dock="Top"`，实际占用高度由
`DockExtent` 控制；单独修改公共 `Height` 不会替代 DockTree 的边缘 extent。
Toolbar 只把 `Menu` 子项标记为顶层菜单，但类型检查不会拒绝其他子节点。

### `Scroll`

只接受 L + S。它没有根矩形，因此不接受 V。声明子节点会被重定向到内部
Content，Viewport 与滚动条是不可直接声明配置的内部节点。

### `Panel`

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Icon` / `TitleIcon` | 标题或页签图标 | 与 Image `Source` 相同的规范/兼容 URI |
| `Title` | 标题栏文字；入组后同步更新对应 Tab | 字符串 |
| `TitleHeight` | 独立 Panel 内部标题栏高度 | 浮点数；当前未夹紧，建议 `>= 0` |
| `TitleColor` | 独立 Panel 标题栏填充色 | 合法颜色 |
| `TitleTextColor` | 独立 Panel 标题文字颜色 | 合法颜色 |

Panel 还接受 L + V + D + R + S + K。启用 Dock 的独立 Panel 在索引阶段会被
包装成单页 PanelGroup：`Dock`、尺寸、最小/最大尺寸和 Margin 会作用到 Group；
`Title` 与图标会同步到 Tab。Group 内的 Panel 原生标题栏会折叠，页签拖动由
`PanelGroupTabNode` 的手势负责，因此 `DragRegion` 描述的是 Panel 自身的 Drag
Behavior，不会把 PanelGroup 空白标题区变成 Dock 入口。

### `PanelGroup`

接受 L + V + R + K。XAML 属性在子 Panel 创建前解析，因此 `<PanelGroup>` 的
开始标签不能使用 `Title`、`Icon`、Scroll 或 Drag 属性；这些属性应写到每个
子 `<Panel>` 上。`ActivePanel`、关闭请求和页签顺序也没有声明属性。

运行时直接调用 `SetProperty` 时，如果 Group 恰好只有一个 Panel，Group 会把
自身不认识的属性继续转发给该 Panel。这是单页包装的兼容行为，不扩展
`<PanelGroup>` 开始标签的合法 XAML 属性集合，也不会恢复 Group 标题栏 Dock。

### `Scene`

| 属性 | 含义 | 合法值与约束 |
| --- | --- | --- |
| `Title` | 场景视口标题 | 字符串 |
| `TitleHeight` | 标题栏高度 | 解析后夹紧到 `>= 1` |

Scene 接受 L + D + R + K。它没有根 `UIObject`，所以不接受 V、`TitleColor` 或
`TitleTextColor`；需要改变标题视觉时应修改 Panel Theme。Scene 默认
`Dock="Fill"`，但可以显式覆盖。

### `Terminal`

Terminal 继承 Panel，因此接受 Panel 的全部属性组和专属属性。它没有额外的
声明属性；`MaxLines`、追加消息和清空消息只通过 C++ API 控制。

## ImmediateUI 强类型属性

ImmediateUI 不把字符串属性全部复制为函数参数，而是使用控件业务参数加统一
`UIStyle`。`UIStyle` 当前包含：

| 字段 | 含义 | 可用范围 |
| --- | --- | --- |
| `Width`、`Height` | 首选尺寸 | 所有节点；完成交互缩放/拖动的 Panel/Group/Scene 会保留交互几何 |
| `MinWidth`、`MinHeight` | 最小尺寸 | 所有节点 |
| `FlexGrow`、`FlexShrink` | Flex 权重 | 所有节点 |
| `Margin`、`Padding` | 四边统一间距 | 所有节点 |
| `Direction` | `FlexDirection::Row/Column` | 所有节点 |
| `Color` | 根填充色 | 仅 DrawNode 派生控件 |
| `BorderColor`、`BorderWidth` | 根边框 | 仅 DrawNode 派生控件 |
| `CornerRadius` | 根圆角 | 仅 DrawNode 派生控件 |

ImmediateUI 当前提供 `BeginPanel`、`Terminal`、`Rect`、`Image`、`Scene`、
`Button`、`Toggle`、`Slider`、`TextInput`、`BeginToolBar`、`BeginMenu` 和
`MenuItem`。业务值通过强类型参数传入，例如 Slider 的 value/minimum/maximum、
Toggle 的 `bool&`、TextInput 的 `std::string&`。它目前没有 Text、Scroll、
TreeView、TreeItem 或显式 PanelGroup 的强类型声明函数。

## 当前未暴露为声明属性的常见字段

以下名称经常会被误认为合法 XAML 属性，但当前实现会拒绝：

| 名称 | 当前状态 |
| --- | --- |
| `Visible`、`Visibility` | 仅 C++ 状态；Menu/Tree 的内部可见性由 `Open/Expanded` 控制 |
| `Position`、`Left`、`Top`、`Right`、`Bottom` | 仅内部/C++ `LayoutStyle` |
| `WidthPercent` | 仅内部/C++ `LayoutStyle` |
| `FontFamily` | TextNode C++ 字段，未接入 SetProperty |
| `MaxLines` | TerminalNode C++ 字段，未接入 SetProperty |
| `ActivePanel` | PanelGroupNode C++ 状态，未接入 SetProperty |
| `SelectedItem` | TreeViewNode C++ 状态，未接入 SetProperty |
| `TopLevel` | Menu 根据父节点自动推导 |
| `DockResizable` | DockProperty 内部字段，未接入 SetProperty |
| `HorizontalOffset`、`VerticalOffset` | ScrollBehavior 运行时状态，未接入 SetProperty |
