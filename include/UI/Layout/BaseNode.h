//
// Created by zhou_zhengming on 2026/7/29.
//

#pragma once
#include "yoga/YGNode.h"

namespace z8 {
class UIObject;
}

namespace z8::ui {
class BaseNode {
public:
  YGNodeRef Node;
  UIObject* O;

  BaseNode();
  virtual ~BaseNode();
  size_t GetChildCount() const;
  void AddChild(BaseNode *child) const;
};

}



