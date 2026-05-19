//
// Created by zhou_zhengming on 2026/5/15.
//

#pragma once
#include "Object.h"

namespace z8
{
class Camera : public Object{
private:
  DirectX::XMFLOAT3 Target;
  DirectX::XMFLOAT3 Up;
  DirectX::XMFLOAT4X4 View;
  DirectX::XMFLOAT4X4 Proj;

  inline static float Near = 1.0f;
  inline static float Far = 1000.0f;
  inline static float Fov = 45.0f;
public:
  Camera();
  DirectX::XMFLOAT4X4& GetView() { return View; }
  DirectX::XMFLOAT4X4& GetProj() { return Proj; }
  void UpdateView();
  void UpdateProj(float aspect);
  void UpdateTarget();
};
}





