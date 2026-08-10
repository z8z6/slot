//
// Created by zhou_zhengming on 2026/7/31.
//

#pragma once


#include "RectNode.h"

#include <string>

namespace z8::ui {
class PanelNode : public RectNode {
public:
  RectNode* TitleNode;
  BaseNode* ContentNode;
  std::string Title;

  PanelNode();
  BaseNode* ContentHost() override;
  const char* TypeName() const override { return "Panel"; }
  bool SetProperty(const std::string& name, const std::string& value) override;
};
}
