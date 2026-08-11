#include "UI/Behavior/ScrollBehavior.h"

#include "UI/Layout/BaseNode.h"
#include "UI/Layout/ScrollBarNode.h"
#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"

#include <algorithm>
#include <cstdlib>

using namespace z8::ui;

namespace {

bool ParseBoolean(const std::string &value, bool &result) {
  if (value == "true" || value == "True" || value == "1") {
    result = true;
    return true;
  }
  if (value == "false" || value == "False" || value == "0") {
    result = false;
    return true;
  }
  return false;
}

bool ParseVisibility(const std::string &value, ScrollBarVisibility &result) {
  if (value == "Hidden")
    result = ScrollBarVisibility::Hidden;
  else if (value == "Auto")
    result = ScrollBarVisibility::Auto;
  else if (value == "Visible")
    result = ScrollBarVisibility::Visible;
  else
    return false;
  return true;
}

} // namespace

void ScrollBehavior::BindVertical(BaseNode *viewport, BaseNode *content,
                                  ScrollBarNode *scrollBar) {
  if (VerticalScrollBar && VerticalScrollBar != scrollBar)
    VerticalScrollBar->ValueChanged = {};
  Viewport = viewport;
  Content = content;
  VerticalScrollBar = scrollBar;
  if (VerticalScrollBar) {
    // 滚动条只发布规范化后的 value，不需要知道内容节点或宿主控件类型。
    VerticalScrollBar->ValueChanged = [this](float value) {
      SetOffsetY(value);
    };
  }
  ApplyProperties();
}

void ScrollBehavior::OnDetached() {
  // 回调捕获 this；卸载时主动断开，允许保留视觉子树并替换滚动策略。
  if (VerticalScrollBar)
    VerticalScrollBar->ValueChanged = {};
  Viewport = nullptr;
  Content = nullptr;
  VerticalScrollBar = nullptr;
}

void ScrollBehavior::SetProperties(const ScrollProperty &properties) {
  Properties = properties;
  Properties.WheelStep = (std::max)(1.0f, Properties.WheelStep);
  ApplyProperties();
  UpdateRange();
}

void ScrollBehavior::ApplyProperties() {
  if (!Viewport)
    return;
  const bool scrolls =
      Properties.Enabled && (Properties.Horizontal || Properties.Vertical);
  // Yoga 的 overflow 只控制测量语义；渲染裁剪和输入命中仍消费同一个
  // ClipsChildren 标志，确保视觉与交互区域不会分离。
  YGNodeStyleSetOverflow(Viewport->GetYogaNode(),
                         scrolls ? YGOverflowScroll : YGOverflowVisible);
  Viewport->SetClipsChildren(scrolls);
  if (VerticalScrollBar) {
    VerticalScrollBar->SetVisible(false);
    if (VerticalScrollBar->ThumbNode)
      VerticalScrollBar->ThumbNode->SetVisible(false);
  }
}

void ScrollBehavior::UpdateRange() {
  if (!Viewport || !Content)
    return;
  float contentExtent = 0.0f;
  for (const auto &child : Content->GetChildren()) {
    const auto yogaNode = child->GetYogaNode();
    contentExtent =
        (std::max)(contentExtent, YGNodeLayoutGetTop(yogaNode) +
                                      YGNodeLayoutGetHeight(yogaNode));
  }
  const float viewportExtent = Viewport->GetLayoutHeight();
  MaximumOffsetY = Properties.Enabled && Properties.Vertical
                       ? (std::max)(0.0f, contentExtent - viewportExtent)
                       : 0.0f;
  OffsetY = std::clamp(OffsetY, 0.0f, MaximumOffsetY);

  if (VerticalScrollBar)
    VerticalScrollBar->SetMetrics(viewportExtent,
                                  viewportExtent + MaximumOffsetY);
  SynchronizeVisuals();
}

void ScrollBehavior::SynchronizeVisuals() {
  if (Viewport)
    Viewport->SetChildOffset(0.0f, -OffsetY);
  if (!VerticalScrollBar)
    return;
  VerticalScrollBar->SetValue(OffsetY, false);

  const auto visibility = Properties.VerticalScrollBar;
  const bool visible =
      Properties.Enabled && Properties.Vertical &&
      (visibility == ScrollBarVisibility::Visible ||
       (visibility == ScrollBarVisibility::Auto && MaximumOffsetY > 0.0f));
  VerticalScrollBar->SetVisible(visible);
  if (VerticalScrollBar->ThumbNode)
    VerticalScrollBar->ThumbNode->SetVisible(visible);
}

void ScrollBehavior::SetOffsetY(float offset) {
  OffsetY = std::clamp(offset, 0.0f, MaximumOffsetY);
  // 即使 offset 数值未变，range 或视觉绑定也可能刚刚更新，必须重新同步。
  SynchronizeVisuals();
}

bool ScrollBehavior::OnMouseWheel(MouseWheelArgs args) {
  if (!Viewport || !Properties.Enabled || !Properties.Vertical ||
      !Viewport->Contains(static_cast<float>(args.X),
                          static_cast<float>(args.Y)) ||
      MaximumOffsetY <= 0.0f)
    return false;
  const float notches =
      static_cast<float>(args.Delta) / static_cast<float>(WHEEL_DELTA);
  SetOffsetY(OffsetY - notches * Properties.WheelStep);
  return true;
}

void ScrollBehavior::OnLayoutUpdated() { UpdateRange(); }

bool ScrollBehavior::SetProperty(const std::string &name,
                                 const std::string &value) {
  bool parsed = false;
  if (name == "Scrollable" || name == "ScrollEnabled")
    parsed = ParseBoolean(value, Properties.Enabled);
  else if (name == "HorizontalScrollEnabled")
    parsed = ParseBoolean(value, Properties.Horizontal);
  else if (name == "VerticalScrollEnabled")
    parsed = ParseBoolean(value, Properties.Vertical);
  else if (name == "HorizontalScrollBar")
    parsed = ParseVisibility(value, Properties.HorizontalScrollBar);
  else if (name == "VerticalScrollBar")
    parsed = ParseVisibility(value, Properties.VerticalScrollBar);
  else if (name == "ShowHorizontalScrollBar") {
    bool visible = false;
    parsed = ParseBoolean(value, visible);
    if (parsed)
      Properties.HorizontalScrollBar =
          visible ? ScrollBarVisibility::Visible : ScrollBarVisibility::Hidden;
  } else if (name == "ShowVerticalScrollBar") {
    bool visible = false;
    parsed = ParseBoolean(value, visible);
    if (parsed)
      Properties.VerticalScrollBar =
          visible ? ScrollBarVisibility::Visible : ScrollBarVisibility::Hidden;
  } else if (name == "WheelStep") {
    Properties.WheelStep =
        (std::max)(1.0f, std::strtof(value.c_str(), nullptr));
    parsed = true;
  } else {
    return false;
  }

  if (parsed) {
    ApplyProperties();
    UpdateRange();
  }
  return parsed;
}
