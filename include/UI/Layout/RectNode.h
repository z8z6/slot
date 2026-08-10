//
// Created by zhou_zhengming on 2026/7/29.
//

#pragma once
#include "BaseNode.h"

namespace z8::ui {
class RectNode : public BaseNode {
public:
  RectNode();
  const char* TypeName() const override { return "Rect"; }
};

}



