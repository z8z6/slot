//
// Created by zhou_zhengming on 2026/5/13.
//

#include "UI/Object/RotateCube.h"

#include <iostream>

#include "UI/Material/RotateCubeMaterial.h"
#include "Util/Math.h"

using namespace z8;
using namespace DirectX;

RotateCube::RotateCube() : LastPos(), objConstants()
{
  Transform.Theta = 1.5f * XM_PI;
  Transform.Phi = XM_PIDIV4;
  Transform.Radius = 5.0f;

  Transform.UpdateCartesian();
  Transform.UpdateWorld();

  Material = new RotateCubeMaterial();
}

void RotateCube::Update(const XMFLOAT4X4& View, const XMFLOAT4X4& Proj)
{
  Transform.UpdateWorldViewProj(View, Proj);
  XMMATRIX wvp = XMLoadFloat4x4(&Transform.WorldViewProj);
  XMStoreFloat4x4(&objConstants, XMMatrixTranspose(wvp));
}


void z8::RotateCube::OnMouseMove(ButtonEventArgs Args)
{
  if ((Args.State & MK_LBUTTON) != 0)
  {
    // Make each pixel correspond to a quarter of a degree.
    float dx = XMConvertToRadians(0.5f * static_cast<float>(Args.X - LastPos.x));
    float dy = XMConvertToRadians(0.5f * static_cast<float>(Args.Y - LastPos.y));

    // Update angles based on input to orbit camera around box.
    Transform.Theta += dx;
    Transform.Phi += dy;

    // Restrict the angle mPhi.
    Transform.Phi = Math::Clamp(Transform.Phi, 0.1f, XM_PI - 0.1f);
    Transform.UpdateCartesian();
    Transform.UpdateWorld();

    std::cout << Transform << "\n";
  }
  else if ((Args.State & MK_RBUTTON) != 0)
  {
    // Make each pixel correspond to 0.005 unit in the scene.
    float dx = 0.05f * static_cast<float>(Args.X - LastPos.x);
    float dy = 0.05f * static_cast<float>(Args.Y - LastPos.y);

    // Update the camera radius based on input.
    Transform.Radius += dx - dy;

    // Restrict the radius.
    Transform.Radius = Math::Clamp(Transform.Radius, 3.0f, 15.0f);
    Transform.UpdateCartesian();
    Transform.UpdateWorld();

    std::cout << Transform << "\n";
  }

  LastPos.x = Args.X;
  LastPos.y = Args.Y;
}

void* RotateCube::ConstBuf()
{
  return &objConstants;
}

unsigned RotateCube::ConstBufSize()
{
  return sizeof(XMFLOAT4X4);
}

