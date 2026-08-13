#include "UI/Layout/ImageNode.h"

#include "Object/UIObject/RectUIObject.h"
#include "UI/Style/Theme.h"

using namespace z8::ui;

ImageNode::ImageNode() : DrawNode(std::make_unique<z8::RectUIObject>()) {
  const auto &theme = Theme::Default();
  Style.Width = 18.0f;
  Style.Height = 18.0f;
  Style.MinWidth = 0.0f;
  Style.MinHeight = 0.0f;
  Style.Margin = 0.0f;
  Style.Padding = 0.0f;
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 0.0f;
  SetColor(theme.Text.MutedColor);
  SetBorder({0.0f, 0.0f, 0.0f, 0.0f}, 0.0f);
  UO->SetImageKind(static_cast<float>(Kind));
}

bool ImageNode::SetProperty(const std::string &name,
                            const std::string &value) {
  if (name == "Tint" || name == "TintColor")
    return DrawNode::SetProperty("Color", value);
  if (name != "Source")
    return DrawNode::SetProperty(name, value);

  ImageKind kind;
  if (value == "builtin://icon/close" ||
      value == "asset://texture/icons/lucide/x.svg")
    kind = ImageKind::Close;
  else if (value == "builtin://icon/plus" ||
           value == "asset://texture/icons/lucide/plus.svg")
    kind = ImageKind::Plus;
  else if (value == "builtin://icon/chevron-down" ||
           value == "asset://texture/icons/lucide/chevron-down.svg")
    kind = ImageKind::ChevronDown;
  else if (value == "builtin://icon/cube" ||
           value == "asset://texture/icons/lucide/box.svg")
    kind = ImageKind::Cube;
  else if (value == "asset://texture/icons/lucide/terminal.svg")
    kind = ImageKind::Terminal;
  else if (value == "asset://texture/icons/lucide/settings-2.svg")
    kind = ImageKind::Settings;
  else
    return false;
  Source = value;
  Kind = kind;
  UO->SetImageKind(static_cast<float>(Kind));
  return true;
}
