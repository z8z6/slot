//
// Created by zhou_zhengming on 2026/7/31.
//

#include "UI/Layout/PanelNode.h"

#include "UI/Object/UIObject/RectUIObject.h"
#include "yoga/YGNodeStyle.h"

#include <cstdlib>

using namespace z8::ui;

PanelNode::PanelNode() : TitleNode(nullptr), ContentNode(nullptr) {
  // Panel 自身纵向排列；标题栏固定高度，内容宿主占据剩余空间。
  YGNodeStyleSetFlexDirection(GetYogaNode(), YGFlexDirectionColumn);
  static_cast<RectUIObject*>(GetUO())->SetColor({0.12f, 0.12f, 0.14f, 0.96f});

  auto title = std::make_unique<RectNode>();
  TitleNode = title.get();
  TitleNode->Key = "__title";
  static_cast<RectUIObject*>(TitleNode->GetUO())->SetColor({0.22f, 0.24f, 0.30f, 1.0f});
  YGNodeStyleSetHeight(TitleNode->GetYogaNode(), 32.0f);
  YGNodeStyleSetFlexGrow(TitleNode->GetYogaNode(), 0.0f);
  YGNodeStyleSetFlexShrink(TitleNode->GetYogaNode(), 0.0f);
  BaseNode::AddChild(std::move(title));

  auto content = std::make_unique<BaseNode>();
  ContentNode = content.get();
  ContentNode->Key = "__content";
  YGNodeStyleSetFlexGrow(ContentNode->GetYogaNode(), 1.0f);
  YGNodeStyleSetFlexShrink(ContentNode->GetYogaNode(), 1.0f);
  BaseNode::AddChild(std::move(content));
}

BaseNode* PanelNode::ContentHost() { return ContentNode; }

bool PanelNode::SetProperty(const std::string& name, const std::string& value) {
  if (name == "Title") {
    // 当前渲染器尚无文字栅格化，先保留标题语义；标题栏几何已经可见。
    Title = value;
    return true;
  }
  if (name == "TitleHeight") {
    YGNodeStyleSetHeight(TitleNode->GetYogaNode(), std::strtof(value.c_str(), nullptr));
    return true;
  }
  return RectNode::SetProperty(name, value);
}
