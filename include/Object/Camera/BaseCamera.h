//
// Created by zhou_zhengming on 2026/5/15.
//

#pragma once
#include "Object/Object.h"

namespace z8
{
class BaseCamera : public Object {
private:
  DirectX::XMFLOAT3 Target;
  DirectX::XMFLOAT3 Up;
  DirectX::XMFLOAT4X4 View;
  DirectX::XMFLOAT4X4 Proj;
  DirectX::XMFLOAT4X4 ViewProj;

  inline static float Near = 1.0f;
  inline static float Far = 1000.0f;
  // 编辑器相机采用中等垂直视场角；超广角会夸大观察方向上的近远透视差，
  // 使比例正确的物体看起来沿 Z 轴被拉长。
  inline static float Fov = 60.0f;
public:
  BaseCamera();
  DirectX::XMFLOAT4X4& GetView() { return View; }
  DirectX::XMFLOAT4X4& GetProj() { return Proj; }
  DirectX::XMFLOAT4X4& GetViewProj() { return ViewProj; }
  void Update(Timer *) override;
  void UpdateView();
  void UpdateProj(float aspect);
  void UpdateViewProj();
  void UpdateTarget();
};
}





