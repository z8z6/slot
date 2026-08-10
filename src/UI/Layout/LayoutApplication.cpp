#include "UI/Layout/Layout.h"

#include "Core/Application.h"

using namespace z8::ui;

void Layout::Update() {
  // Window 依赖被隔离在适配层，布局、XAML 和声明 API 可在无窗口测试中使用。
  Calculate(static_cast<float>(App->Window.Width),
            static_cast<float>(App->Window.Height));
}
