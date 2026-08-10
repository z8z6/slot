#include "UI/Declarative/ControlFactory.h"

#include "UI/Layout/BaseNode.h"
#include "UI/Layout/PanelNode.h"
#include "UI/Layout/RectNode.h"

using namespace z8::ui;

ControlFactory::ControlFactory() {
  Register("UI", [] { return std::make_unique<BaseNode>(); });
  Register("Rect", [] { return std::make_unique<RectNode>(); });
  Register("Panel", [] { return std::make_unique<PanelNode>(); });
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
