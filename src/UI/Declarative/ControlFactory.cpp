#include "UI/Declarative/ControlFactory.h"

#include "UI/Layout/BaseNode.h"
#include "UI/Layout/BehaviorNode.h"
#include "UI/Layout/ButtonNode.h"
#include "UI/Layout/ImageNode.h"
#include "UI/Layout/MenuNode.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Layout/PanelNode.h"
#include "UI/Layout/RectNode.h"
#include "UI/Layout/SceneNode.h"
#include "UI/Layout/ScrollNode.h"
#include "UI/Layout/SliderNode.h"
#include "UI/Layout/TerminalNode.h"
#include "UI/Layout/TextInputNode.h"
#include "UI/Layout/TextNode.h"
#include "UI/Layout/ToggleNode.h"
#include "UI/Layout/ToolBarNode.h"
#include "UI/Layout/TreeViewNode.h"

using namespace z8::ui;

ControlFactory::ControlFactory() {
  // 根 UI 节点需要 DockSpace 行为；普通结构节点仍直接使用纯 BaseNode。
  Register("UI", [] { return std::make_unique<BehaviorNode>(); });
  Register("Rect", [] { return std::make_unique<RectNode>(); });
  Register("Image", [] { return std::make_unique<ImageNode>(); });
  Register("Text", [] { return std::make_unique<TextNode>(); });
  Register("Button", [] { return std::make_unique<ButtonNode>(); });
  Register("Toggle", [] { return std::make_unique<ToggleNode>(); });
  Register("Slider", [] { return std::make_unique<SliderNode>(); });
  // 保留用户常用的 Slide 拼写作为声明别名，运行时类型仍统一为 Slider。
  Register("Slide", [] { return std::make_unique<SliderNode>(); });
  Register("TextInput", [] { return std::make_unique<TextInputNode>(); });
  Register("TreeView", [] { return std::make_unique<TreeViewNode>(); });
  Register("TreeItem", [] { return std::make_unique<TreeViewItemNode>(); });
  Register("Menu", [] { return std::make_unique<MenuNode>(); });
  Register("MenuItem", [] { return std::make_unique<MenuItemNode>(); });
  Register("ToolBar", [] { return std::make_unique<ToolBarNode>(); });
  Register("Toolbar", [] { return std::make_unique<ToolBarNode>(); });
  Register("Terminal", [] { return std::make_unique<TerminalNode>(); });
  Register("Scroll", [] { return std::make_unique<ScrollNode>(); });
  Register("Scene", [] { return std::make_unique<SceneNode>(); });
  Register("Panel", [] { return std::make_unique<PanelNode>(); });
  Register("PanelGroup", [] { return std::make_unique<PanelGroupNode>(); });
}

void ControlFactory::Register(const std::string &type, Creator creator) {
  Creators[type] = std::move(creator);
}

std::unique_ptr<BaseNode>
ControlFactory::Create(const std::string &type) const {
  const auto it = Creators.find(type);
  return it == Creators.end() ? nullptr : it->second();
}

ControlFactory &ControlFactory::Instance() {
  static ControlFactory factory;
  return factory;
}
