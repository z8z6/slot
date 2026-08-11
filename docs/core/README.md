# 应用核心

## 启动和主循环

1. `WinMain` 设置 `Window::Instance`，构造 `GameApplication`。
2. `Window` 创建 HWND，`Application` 将窗口过程转发到实例的 `MsgHandler`。
3. `GameApplication::Init` 创建第一人称相机、方向光、场景和 UI。
4. 创建并初始化 `DX12Render`，然后显示窗口。
5. `Application::Run` 处理消息；空闲时逐应用执行：

```text
Timer.Tick → ShowFrame → Layout.Update → Render.Update → Render.Draw
```

当前示例创建五个缩放立方体和三个矩形 UI 子节点。`ShowFrame` 每秒更新一次窗口标题中的 FPS 和毫秒/帧。

## 窗口与输入

窗口默认客户区为 960×540。Resize 时先更新宽高和 Yoga 布局，再让后端重建交换链/深度资源、viewport、scissor 和相机投影。交互拉伸期间只更新布局和发送事件，退出 Win32 拖拉模态循环后再统一重建 GPU 资源，避免每个 `WM_SIZE` 都等待交换链。

鼠标键盘事件包装为 `MouseMovArgs`/`KeyArgs`，窗口移动与拉伸事件包装为 `WindowMoveArgs`/`WindowResizeArgs`，然后广播给 UI Object、场景对象、相机和灯光。每个 `Object` 可覆写鼠标移动、按下、抬起、拖拽，键盘，以及窗口移动、交互拖动、尺寸变化、交互拉伸和拖拉循环起止回调。鼠标位移和窗口状态按 `Application` 独立保存，多窗口不会串扰；按下鼠标后窗口会捕获指针，拖出客户区仍可正确收到拖拽和抬起事件。

UI 指针事件先按逆绘制顺序命中最上层视觉节点，再沿父节点冒泡；开始手势的节点会捕获后续拖拽和抬起事件，且 UI 已消费的指针事件不会穿透到相机或场景。`WM_SETCURSOR` 会把控件请求的平台无关指针形状映射为系统 DPI 感知光标。键盘焦点和 XAML 事件绑定仍待后续补充。第一人称相机支持鼠标转向、WASD、Space/Shift；数字键 1/2 切换实体/线框。

## 源码入口

- `src/main.cpp`
- `src/Core/Application.cpp`
- `src/Core/GameApplication.cpp`
- `src/Core/Window.cpp`
- `include/Core/Event.h`
