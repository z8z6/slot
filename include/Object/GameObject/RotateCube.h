//
// Created by zhou_zhengming on 2026/5/13.
//

#pragma once
#include "CubeObject.h"

namespace z8
{
class RotateCube : public CubeObject
{
public:
  RotateCube();
  EventReply OnMouseMove(MouseMovArgs) override;
};
}
