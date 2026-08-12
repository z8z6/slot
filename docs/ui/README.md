# UI 层架构

UI 层采用“多种声明前端 + 单一保留式控件树 + Yoga 布局 + D3D12 渲染适配”的结构。XAML 与 ImGui 风格 API 不是两套控件系统：二者最终都生成 `BaseNode` 控件树，因此控件、布局规则和渲染行为保持一致。

```mermaid
flowchart LR
    XAML[XamlLoader] --> Factory[ControlFactory]
    Immediate[ImmediateUI] --> Factory
    Factory --> Property[IProperty 属性协议]
    Property --> Tree[BaseNode 控件树]
    Tree --> Yoga[Yoga 布局树]
    Tree --> Index[Layout 扁平渲染索引]
    Yoga --> Transform[UIObject Transform]
    Index --> Batch[DX12 UI RenderBatch]
```

## 控件树和所有权

`BaseNode` 只通过 `unique_ptr` 独占子节点，负责 Yoga 几何、树关系、属性和裁剪传播；Root、Viewport、Content 等结构节点因此不携带交互状态。`BehaviorNode : BaseNode` 选择性加入事件与 Behavior，`DrawNode : BehaviorNode` 则以 `unique_ptr<UIObject> UO` 建立渲染所有权边界，`RectNode` 及其派生控件继承绘制层。

每个节点包含稳定 `Key`、父节点和 Yoga Node。`ContentHost()` 决定声明的子控件实际进入哪里；普通容器返回自身，Panel 返回内部滚动内容区。`Layout` 在拓扑变化时生成全部节点 `Nodes`、几何视觉 `Visuals` 和文字 `Texts` 三份非拥有索引；每帧布局通过 `Synchronize()` 窄虚接口提交位置、缩放和裁剪。

框架内部直接访问 `Node`、`Left/Top/Width/Height`、`Parent`、`Children`、`Visible` 等简单状态，不再用只返回成员的 Getter/Setter 包装。带边界语义的操作仍保留函数，例如 `AddChild()` 必须同步 Yoga 树，`SetProperty()` 负责声明解析，`GetBehavior<T>()` 负责运行时能力查询。

## EventTarget 与 Behavior 组合

场景 `Object`、`Layout`、`BehaviorNode` 和 `IBehavior` 共同继承 Core 的 `EventTarget`，鼠标与键盘事件统一返回 `EventReply::Ignored/Handled/Capture`。`BaseNode` 只负责 Yoga、属性与树所有权；纯布局节点因此不再支付 Behavior 容器、事件虚表和捕获状态的成本。Core 只定义事件 ABI，不负责命中、冒泡或捕获存储。

`PanelNode` 由背景视觉、`TextNode` 标题和 `ScrollNode` 内容区组成。`ScrollNode` 独立封装 viewport/content/scrollbar 与 `ScrollBehavior`，可被列表和树控件复用。`TextNode` 只保存 UTF-8 文本与排版属性，布局完成后由 DirectWrite 文字通道在 DX12 UI 几何之上绘制；D3D11On12 负责共享交换链缓冲和同步，文字不伪装成矩形网格资源。

`BaseNode` 实现属性根接口 `IProperty`，因此 XAML、即时声明和未来检查器只需面向统一的 `SetProperty` 协议。可选行为以 `unique_ptr<IBehavior>` 挂载到 `BehaviorNode`，`DragBehavior`、`ResizeBehavior`、`ScrollBehavior`、`DockBehavior` 分别独占自己的配置和运行时状态；交互宿主销毁时先取消捕获并释放行为，再释放视觉子树和 Yoga 几何。

行为按优先级稳定排序。`ResizeBehavior` 高于 `DragBehavior`，所以标题栏边缘只会开始 Resize；同优先级保持声明顺序。按下结果使用 `Ignored/Handled/Capture` 三态，避免把“消费一次点击”和“捕获一段手势”混为同一个 `bool`。声明属性会自动遍历行为链，因此给普通 Rect 挂载 Drag 后，`Draggable`、`DragRegion` 立即可由 XAML/检查器设置，不需要修改 Rect 类型：

```cpp
auto rect = std::make_unique<z8::ui::RectNode>();
auto* drag = rect->AddBehavior<z8::ui::DragBehavior>();
drag->Properties.Region = z8::ui::DragRegion::Anywhere;
rect->AddBehavior<z8::ui::ResizeBehavior>();
```

`IDraggable`、`IResizable`、`IScrollable` 已删除。能力发现统一使用 `GetBehavior<T>()`，添加能力统一使用 `AddBehavior<T>()`，Panel 不再保留转发 getter 或接口子对象。简单配置公开为 Behavior 的 `Properties`；会影响 Yoga 约束或滚动范围的批量修改仍调用 `SetProperties()` 维护不变量。由此形成四个明确边界：Style 描述外观和盒模型，Property 是可序列化配置，Behavior 处理输入和短期状态，Control 只组装视觉树及行为。

## XAML 声明

```cpp
z8::ui::XamlLoader loader;
auto result = loader.LoadFileInto(Layout, "ui/Main.xaml");
if (!result) {
  std::cerr << result.Error << " at " << result.ErrorOffset << '\n';
}
```

当前 XAML 子集支持 `UI`、`Panel`、`Rect`，以及嵌套、自闭合标签、XML 声明、注释和常用实体。示例：

```xml
<UI Direction="Column">
  <Panel Id="inspector" Title="Inspector"
         Width="360" Height="240" TitleHeight="32" Padding="8">
    <Rect Id="properties" FlexGrow="1" Margin="4" />
  </Panel>
</UI>
```

通用属性包括 `Id/Key/Name`、`Width/Height`、`MinWidth/MinHeight`、`MaxWidth/MaxHeight`、`FlexGrow/FlexShrink`、`Margin/Padding`、`Color` 和 `Direction="Row|Column"`。`Color` 接受 `#RRGGBB`、`#RRGGBBAA` 或 `r,g,b[,a]`。

Panel 的行为属性位于各 Behavior 的 `Properties` 中，不再与标题、颜色和盒模型字段平铺，也不再经过 Panel 转发。声明属性仍由 `BaseNode::SetProperty()` 自动遍历行为链：

| 属性组 | XAML 属性 | 含义 |
| --- | --- | --- |
| Drag | `Draggable`/`DragEnabled` | 是否允许移动 Panel |
| Drag | `DragRegion="TitleBar\|Anywhere"` | 仅标题栏或任意内部区域可启动拖动 |
| Resize | `Resizable`/`ResizeEnabled` | 是否允许从边和角拉伸 |
| Resize | `ResizeBorder` | 拉伸命中区域宽度 |
| Scroll | `Scrollable`/`ScrollEnabled` | 滚动行为总开关 |
| Scroll | `HorizontalScrollEnabled`、`VerticalScrollEnabled` | 分方向允许滚动 |
| Scroll | `HorizontalScrollBar`、`VerticalScrollBar` | `Hidden`、`Auto` 或 `Visible` |
| Scroll | `ShowHorizontalScrollBar`、`ShowVerticalScrollBar` | 显式显示/隐藏滚动条的布尔便捷属性 |
| Scroll | `WheelStep` | 单次滚轮输入的逻辑滚动距离 |
| Dock | `DockEnabled` | 是否参与自动布局与边缘吸附 |
| Dock | `Dock="Auto\|Floating\|Left\|Right\|Top\|Bottom\|Fill"` | 自动布局、浮动或停靠方向 |
| Dock | `DockThreshold`、`DockExtent` | 边缘吸附距离与停靠尺寸 |

例如：

```xml
<Panel DragRegion="Anywhere"
       Scrollable="true"
       HorizontalScrollEnabled="false"
       VerticalScrollEnabled="true"
       HorizontalScrollBar="Hidden"
       VerticalScrollBar="Auto" />
```

加载器对未知控件、未知属性、标签不匹配和不支持的文本节点返回带偏移的错误。控件创建通过 `ControlFactory` 注册；添加新控件类型不需要修改解析器：

```cpp
ControlFactory::Instance().Register("MyControl", [] {
  return std::make_unique<MyControlNode>();
});
```

当前是刻意收敛的 XAML 子集，尚不支持 XML 命名空间、属性元素、数据绑定、资源字典和文本内容。

## ImGui 风格声明

```cpp
z8::ui::ImmediateUI ui(Layout);
ui.BeginFrame();

z8::ui::UIStyle panelStyle;
panelStyle.Width = 360.0f;
panelStyle.Height = 240.0f;
panelStyle.Padding = 8.0f;

if (ui.BeginPanel("inspector", "Inspector", panelStyle)) {
  z8::ui::UIStyle row;
  row.FlexGrow = 1.0f;
  row.Margin = 4.0f;
  ui.Rect("properties", row);
  ui.EndPanel();
}

ui.EndFrame();
```

API 外观与 ImGui 相似，但内部不是每帧销毁重建。每个作用域按 `key + 控件类型 + 调用顺序` 与上一帧协调：稳定声明直接复用节点、Yoga 缓存和 UIObject；结构发生变化时，只截断第一个差异点之后的同级后缀。

XAML 的显式 Key 在整棵树中必须唯一；即时 API 的 key 必须非空且在当前同级作用域唯一。加载/声明阶段会直接拒绝冲突，避免后续查找和状态复用产生歧义。

未再次声明的控件会在 `EndFrame` 移除。Begin/End 不匹配会通过 `LastError()` 报告。

## Panel

Panel 的 Yoga 内部树固定为：

```text
Panel（可渲染背景，Column）
├── TitleNode（可渲染矩形，固定 TitleHeight）
├── ScrollViewportNode（填充剩余空间并裁剪）
│   └── ContentNode（按内容自然尺寸增长）
│       └── 用户声明的子控件
└── VerticalScrollBarNode（绝对定位的独立复合控件）
    └── ThumbNode
```

标题栏、视口和滚动条是内部节点，调用者的子控件永远被重定向到 ContentNode，不会破坏复合控件结构。Panel 背景和标题栏使用不同对象颜色；UI PSO 关闭深度写入并启用 alpha，保证标题栏按声明顺序覆盖背景。`Title` 已作为控件语义保存，但项目尚无字体/字形渲染器，文字显示需在文字渲染模块完成后接入。

Panel 默认组装 Drag、Resize、Scroll、Dock 四个行为，但自身不再覆写鼠标事件或保存手势状态。根节点默认挂载 `DockLayoutBehavior`：多个 `Dock="Auto"` Panel 横向均分剩余窗口；Left/Right/Top/Bottom 依声明顺序切割空间；Floating 保留用户几何。左键拖动标题栏产生实际位移后会转为 Floating，释放到父容器边缘阈值内则记录新停靠方向，并在下一次布局计算中重排其他 Panel。单击标题栏不会解除停靠。

拖动会把当前流式布局结果固化成 Yoga 绝对定位，随后更新 `left/top`；边缘和四角各有默认 6 像素的缩放命中区。指针位于边界时会显示对应的水平、垂直或对角系统缩放光标。缩放默认限制为 240×160，`MinWidth/MinHeight` 会同时更新交互限制和 Yoga 约束。可通过 `Draggable="false"`、`Resizable="false"` 或 `ResizeBorder` 调整行为。

默认 `Drag.Region` 为 `TitleBar`。默认滚动总开关开启，仅允许垂直方向；水平滚动条为 `Hidden`，垂直滚动条为 `Auto`，滚轮步长为 40。Panel 根据内容范围计算并夹紧偏移；滚轮移动内容，轨道点击按一页移动，滑块拖拽通过指针捕获连续更新 value。`ScrollBarNode` 只管理 range/value 和滑块，不拥有内容，因而可被后续独立 ScrollView、列表和水平滚动复用。

视口裁剪不会为每个子项反复切换 D3D12 scissor。布局遍历把父子裁剪矩形求交后写入每个 `UIObject` 常量，像素着色器在视口外丢弃像素；命中测试使用同一裁剪矩形，因此不可见内容也不会接收事件。这个方案优先保证当前逐对象渲染结构下的一致性，未来批处理器可再按相同 clip 合批并切换硬件 scissor。

## 与 Unity / Qt 的架构对应

当前设计采用两者共有的保留式、组合式思路，而不是复制引擎 API：

- 与 Unity UI Toolkit 的 Manipulator 类似，Behavior 可附着到已有视觉节点，并在一段 Pointer 手势内捕获输入；控件类型不因 Drag、Resize 的排列组合而膨胀。稳定视觉树负责所有权，Yoga/Flexbox 负责布局，ScrollView 明确拆成 viewport、content container 和 scroller。
- 与 Qt 的事件过滤/对象组合及 `QAbstractScrollArea` 类似，输入路由与具体控件实现分离，滚动容器通过 viewport、content、scrollbar 的窄协议协作。Slot 使用强类型 Behavior 代替全局事件过滤器，避免字符串事件和无约束 QObject 转型扩散到渲染核心。
- Slot 保留 XAML 和即时声明两个前端，但二者只协调同一棵 `BaseNode` 树，运行时状态不会因为声明方式不同而分叉。

下一阶段适合在现有边界上增加属性元数据（类型、默认值、序列化名）、伪状态样式、焦点/键盘导航和布局/绘制失效标记，而不是继续扩充 Panel 的职责。

`Layout` 使用与渲染相同的绝对布局框做逆绘制顺序命中，事件从视觉目标向父节点冒泡。节点先按优先级询问 Behavior，再调用共享的 `EventTarget` 节点钩子；捕获精确记录到开始手势的 Behavior。指针移出控件后拖动仍连续，声明式拓扑重建则主动发送 capture-lost 使状态机复位；命中 UI 的事件不会继续影响背后的相机或场景对象。

## 默认主题

`Color` 集中定义 UE 编辑器风格的深灰层级、蓝色强调、文本状态和反馈色；`Theme::UnrealEditor()` 再把这些基础色映射为 Rect、Text、ScrollBar、Panel 与 List 的语义样式。控件构造时应用主题，XAML 或 Immediate UI 属性随后覆盖。`Theme::Modern()` 是默认主题的兼容别名。Demo 只读取 Panel 和 List 默认样式中的尺寸、间距、交替行和非焦点选中色，不再散落颜色常量。

新增控件应先在 `Theme` 中定义对应的样式结构，再在构造函数统一应用 `Color`、`Margin`、`Padding` 和最小尺寸，避免重新引入散落的魔法值。实例属性始终拥有更高优先级。Immediate UI 的 `Style` 参数可以省略；稳定节点会在每帧先恢复主题默认值，再应用本帧显式字段，避免继承上一帧已经移除的覆盖。

## 布局和渲染同步

Yoga 计算 left/top/width/height 后，`Layout` 累加父偏移得到绝对像素坐标。矩形原点在中心，因此位置转换为 `(left + width/2, top + height/2)`，缩放为 `(width, height)`；UI Shader 再将像素坐标映射到 NDC 并翻转 Y。

`Layout` 仅在控件拓扑改变时设置 dirty 标记。`DX12Render` 消费该标记并重建 UI 常量缓冲和 RenderObject 索引；样式或尺寸变化只更新已有常量，不重建 GPU 资源。空 UI 批次现在可安全初始化和绘制。

## 测试

```powershell
cmake --build cmake-build-debug --target slot_ui_tests
ctest --test-dir cmake-build-debug --output-on-failure
```

测试使用 GoogleTest，按 `tests/UI/Controls`、`tests/UI/Layout`、`tests/UI/Declarative` 分类。BaseNode、RectNode、PanelNode 分别拥有独立单元测试，并覆盖属性接口发现、所有权释放、XAML、错误诊断、即时声明复用、拓扑 dirty 状态、Panel 拖动、角落缩放、缩放光标、滚轮偏移、滚动条可见性、视口裁剪、最小尺寸限制和统一颜色覆盖。运行时提示、错误与警告统一使用英文。

## 后续扩展

- 为属性解析增加颜色、百分比、边级 margin/padding 和严格数值诊断。
- 增加 Text 控件、字体图集和批量字形渲染，使 Panel 标题文字可见。
- 建立 UI 专用 PSO，支持 alpha blend、scissor、z-order 和圆角。
- 增加键盘焦点、Tab 导航、悬停/光标反馈及 XAML 事件绑定。
- 对大型界面把同级协调从顺序前缀扩展为 keyed move，降低大范围重排成本。

## 关键源码

- `include/UI/Declarative`、`src/UI/Declarative`
- `include/UI/Layout/BaseNode.h`、`src/UI/Layout/BaseNode.cpp`
- `include/UI/Layout/PanelNode.h`、`src/UI/Layout/PanelNode.cpp`
- `include/UI/Layout/ScrollBarNode.h`、`src/UI/Layout/ScrollBarNode.cpp`
- `include/UI/Behavior`、`src/UI/Behavior`
- `include/UI/Property/IProperty.h`
- `include/UI/Style/Theme.h`
- `src/UI/Layout/Layout.cpp`、`LayoutApplication.cpp`
- `src/Target/DirectX/DX12Render.cpp`、`DX12RenderBatch.cpp`
- `tests/UI/Controls`、`tests/UI/Layout`、`tests/UI/Declarative`
