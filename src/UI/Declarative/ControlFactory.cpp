#include "UI/Declarative/ControlFactory.h"

#include "UI/Layout/BaseNode.h"
#include "UI/Layout/BehaviorNode.h"
#include "UI/Layout/PanelNode.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Layout/RectNode.h"
#include "UI/Layout/ScrollNode.h"
#include "UI/Layout/SceneNode.h"
#include "UI/Layout/TextNode.h"
#include "UI/Layout/TerminalNode.h"

using namespace z8::ui;

ControlFactory::ControlFactory() {
  // 根 UI 节点需要 DockSpace 行为；普通结构节点仍直接使用纯 BaseNode。
  Register("UI", [] { return std::make_unique<BehaviorNode>(); });
  Register("Rect", [] { return std::make_unique<RectNode>(); });
  Register("Text", [] { return std::make_unique<TextNode>(); });
  Register("Terminal", [] { return std::make_unique<TerminalNode>(); });
  Register("Scroll", [] { return std::make_unique<ScrollNode>(); });
  Register("Scene", [] { return std::make_unique<SceneNode>(); });
  Register("Panel", [] { return std::make_unique<PanelNode>(); });
  Register("PanelGroup", [] { return std::make_unique<PanelGroupNode>(); });
}

void ControlFactory::Register(const std::string& type, Creator creator) {
  Creators[type] = std::move(creator);
}

std::unique_ptr<BaseNode> ControlFactory::Create(const std::string& type) const {
  const auto it = Creators.find(type);
  return it == Creators.end() ? nullptr : it->second();
}

ControlFactory& ControlFactory::Instance() {
  static ControlFactory factory;
  return factory;
}
