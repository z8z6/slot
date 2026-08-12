//
// Created by zhou_zhengming on 2026/7/29.
//

#pragma once
#include "DrawNode.h"

namespace z8::ui {
/** 使用矩形网格绘制的基础视觉节点。 */
class RectNode : public DrawNode {
public:
  RectNode();
  const char *TypeName() const override { return "Rect"; }
};
} // namespace z8::ui
