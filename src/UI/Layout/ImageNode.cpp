#include "UI/Layout/ImageNode.h"

#include "Object/UIObject/RectUIObject.h"
#include "UI/Style/Theme.h"

#include <array>
#include <optional>
#include <stdexcept>

using namespace z8::ui;

namespace {

constexpr std::array IconRegistry{
    UIIconInfo{UIIcon::Close, "asset://texture/icons/lucide/x.svg"},
    UIIconInfo{UIIcon::Plus, "asset://texture/icons/lucide/plus.svg"},
    UIIconInfo{UIIcon::ChevronDown,
               "asset://texture/icons/lucide/chevron-down.svg"},
    UIIconInfo{UIIcon::Cube, "asset://texture/icons/lucide/box.svg"},
    UIIconInfo{UIIcon::Terminal, "asset://texture/icons/lucide/terminal.svg"},
    UIIconInfo{UIIcon::Settings, "asset://texture/icons/lucide/settings-2.svg"},
    UIIconInfo{UIIcon::ChevronRight,
               "asset://texture/icons/lucide/chevron-right.svg"}};

std::optional<UIIcon> ResolveIcon(std::string_view source) {
  for (const auto &entry : IconRegistry)
    if (entry.Source == source)
      return entry.Icon;
  if (source == "builtin://icon/close")
    return UIIcon::Close;
  if (source == "builtin://icon/plus")
    return UIIcon::Plus;
  if (source == "builtin://icon/chevron-down")
    return UIIcon::ChevronDown;
  if (source == "builtin://icon/cube")
    return UIIcon::Cube;
  if (source == "builtin://icon/chevron-right")
    return UIIcon::ChevronRight;
  return std::nullopt;
}

} // namespace

const UIIconInfo &z8::ui::GetUIIconInfo(UIIcon icon) {
  for (const auto &entry : IconRegistry)
    if (entry.Icon == icon)
      return entry;
  // UIIcon 是闭合集合；越界值通常意味着 ABI 或未初始化数据损坏，不能静默
  // 替换为另一图标掩盖调用错误。
  throw std::out_of_range("Unknown UI icon.");
}

ImageNode::ImageNode() : DrawNode(std::make_unique<z8::RectUIObject>()) {
  const auto &theme = Theme::Default();
  Style.Width = theme.Icon.NormalSize;
  Style.Height = theme.Icon.NormalSize;
  Style.MinWidth = 0.0f;
  Style.MinHeight = 0.0f;
  Style.Margin = 0.0f;
  Style.Padding = 0.0f;
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 0.0f;
  SetColor(theme.Icon.Color.Resolve(WidgetVisualState::Normal));
  SetBorder({0.0f, 0.0f, 0.0f, 0.0f}, 0.0f);
  UO->SetImageKind(static_cast<float>(Icon));
}

bool ImageNode::SetIcon(UIIcon icon) {
  const auto &entry = GetUIIconInfo(icon);
  Icon = icon;
  Source = entry.Source;
  UO->SetImageKind(static_cast<float>(Icon));
  return true;
}

bool ImageNode::SetProperty(const std::string &name, const std::string &value) {
  if (name == "Tint" || name == "TintColor")
    return DrawNode::SetProperty("Color", value);
  if (name != "Source")
    return DrawNode::SetProperty(name, value);

  const auto icon = ResolveIcon(value);
  if (!icon)
    return false;
  Source = value;
  Icon = *icon;
  UO->SetImageKind(static_cast<float>(Icon));
  return true;
}
