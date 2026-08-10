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

窗口默认客户区为 960×540。Resize 时先更新宽高和 Yoga 布局，再让后端重建交换链/深度资源、viewport、scissor 和相机投影。

鼠标键盘事件包装为 `MouseMovArgs`/`KeyArgs` 后广播给场景对象和相机。第一人称相机支持鼠标转向、WASD、Space/Shift；数字键 1/2 切换实体/线框。UI 尚无命中测试、焦点和事件冒泡。

## 源码入口

- `src/main.cpp`
- `src/Core/Application.cpp`
- `src/Core/GameApplication.cpp`
- `src/Core/Window.cpp`
- `include/Core/Event.h`
