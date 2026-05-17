//
// Created by zhou_zhengming on 2026/5/9.
//

#pragma once

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

  static Render* CreateRender(Application* App, RenderType type = DirectX12);
};
}


