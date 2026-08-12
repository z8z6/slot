//
// Created by zhou_zhengming on 2026/7/29.
//

#pragma once
#include "DrawNode.h"

namespace z8::ui {

class RectNode : public DrawNode {
public:
  RectNode();
  const char *TypeName() const override { return "Rect"; }
};
} // namespace z8::ui
