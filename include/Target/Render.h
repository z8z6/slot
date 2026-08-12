//
// Created by zhou_zhengming on 2026/5/9.
//

#pragma once
#include <memory>

namespace z8 {
class Application;

enum RenderType
{
  DirectX12 = 0,
  DirectX11 = 1,
  Vulkan = 2
};

class Render {
public:
  virtual ~Render() = default;
  virtual void Init() = 0;
  virtual void Update() = 0;
  virtual void Draw() = 0;
  virtual void Resize() = 0;
  /**
   * 在 Application 的 CPU 资源和窗口成员析构前显式停止设备资源。
   * 默认后端没有额外工作；图形后端必须保证重复调用安全。
   */
  virtual void Shutdown() {}

  static std::unique_ptr<Render> CreateRender(Application* App, RenderType type = DirectX12);
};
}


