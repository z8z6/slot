//
// Created by zhou_zhengming on 2026/5/13.
//

#include "UI/Object/GameObject/RotateCube.h"
#include "UI/Mesh/MeshRegistry.h"
#include "Util/Math.h"
#include <iostream>

using namespace z8;
using namespace DirectX;

RotateCube::RotateCube()
{
  Transform.Theta = 1.5f * XM_PI;
  Transform.Phi = XM_PIDIV4;
  Transform.Radius = 5.0f;

  Transform.UpdateCartesian();
}


void z8::RotateCube::OnMouseMove(MouseMovArgs Args) {
  if ((Args.State & MK_LBUTTON) != 0)
  {
    // Make each pixel correspond to a quarter of a degree.
    float dx = XMConvertToRadians(0.5f * static_cast<float>(Args.DeltaX));
    float dy = XMConvertToRadians(0.5f * static_cast<float>(Args.DeltaY));

    // Update angles based on input to orbit camera around box.
    Transform.Theta += dx;
    Transform.Phi += dy;

    // Restrict the angle mPhi.
    Transform.Phi = Math::Clamp(Transform.Phi, 0.1f, XM_PI - 0.1f);
    Transform.UpdateCartesian();
  }
  else if ((Args.State & MK_RBUTTON) != 0)
  {
    // Make each pixel correspond to 0.005 unit in the scene.
    float dx = 0.05f * static_cast<float>(Args.DeltaX);
    float dy = 0.05f * static_cast<float>(Args.DeltaY);

    // Update the camera radius based on input.
    Transform.Radius += dx - dy;

    // Restrict the radius.
    Transform.Radius = Math::Clamp(Transform.Radius, 3.0f, 15.0f);
    Transform.UpdateCartesian();
  }
}



