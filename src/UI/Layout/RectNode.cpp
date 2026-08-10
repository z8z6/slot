//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/RectNode.h"

#include "UI/Object/UIObject/RectUIObject.h"

using namespace z8::ui;

RectNode::RectNode() {
  SetObject(std::make_unique<RectUIObject>());
}
