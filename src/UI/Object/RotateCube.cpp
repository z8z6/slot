//
// Created by zhou_zhengming on 2026/5/13.
//

#include "UI/Object/RotateCube.h"

#include "Util/Math.h"

using namespace z8;
using namespace DirectX;

RotateCube::RotateCube() : LastPos()
{
}

void RotateCube::Update(XMFLOAT4X4 mProj)
{
  XMFLOAT4X4 mWorld = Math::Identity4x4();
  XMFLOAT4X4 mView = Math::Identity4x4();

  // Convert Spherical to Cartesian coordinates.
  float x = Radius*sinf(Phi)*cosf(Theta);
  float z = Radius*sinf(Phi)*sinf(Theta);
  float y = Radius*cosf(Phi);

  // Build the view matrix.
  XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
  XMVECTOR target = XMVectorZero();
  XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

  XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
  XMStoreFloat4x4(&mView, view);

  XMMATRIX world = XMLoadFloat4x4(&mWorld);
  XMMATRIX proj = XMLoadFloat4x4(&mProj);
  WorldViewProj = world*view*proj;

  // Update the constant buffer with the latest worldViewProj matrix.
  XMStoreFloat4x4(&objConstants, XMMatrixTranspose(WorldViewProj));
}

void* RotateCube::ConstBuf()
{
  return &objConstants;
}

unsigned RotateCube::ConstBufSize()
{
  return sizeof(XMFLOAT4X4);
}

void z8::RotateCube::OnMouseMove(ButtonEventArgs Args)
{
  if((Args.State & MK_LBUTTON) != 0)
  {
    // Make each pixel correspond to a quarter of a degree.
    float dx = XMConvertToRadians(0.25f*static_cast<float>(Args.X - LastPos.x));
    float dy = XMConvertToRadians(0.25f*static_cast<float>(Args.Y - LastPos.y));

    // Update angles based on input to orbit camera around box.
    Theta += dx;
    Phi += dy;

    // Restrict the angle mPhi.
    Phi = Math::Clamp(Phi, 0.1f, XM_PI - 0.1f);
  }
  else if((Args.State & MK_RBUTTON) != 0)
  {
    // Make each pixel correspond to 0.005 unit in the scene.
    float dx = 0.005f*static_cast<float>(Args.X - LastPos.x);
    float dy = 0.005f*static_cast<float>(Args.Y - LastPos.y);

    // Update the camera radius based on input.
    Radius += dx - dy;

    // Restrict the radius.
    Radius = Math::Clamp(Radius, 3.0f, 15.0f);
  }

  LastPos.x = Args.X;
  LastPos.y = Args.Y;
}

