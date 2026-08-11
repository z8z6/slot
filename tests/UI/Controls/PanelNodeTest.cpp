#include "UI/Layout/PanelNode.h"
#include "UI/Layout/Layout.h"
#include "UI/Declarative/ImmediateUI.h"

#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"

#include <gtest/gtest.h>

namespace z8::ui {
namespace {

MouseMovArgs MouseArgs(int x, int y, int deltaX = 0, int deltaY = 0) {
  MouseMovArgs args;
  args.X = x;
  args.Y = y;
  args.DeltaX = deltaX;
  args.DeltaY = deltaY;
  args.State = MK_LBUTTON;
  args.Button = MouseButton::Left;
  return args;
}

PanelNode* AddPanel(Layout& layout, float width = 300.0f,
                    float height = 200.0f) {
  auto panel = std::make_unique<PanelNode>();
  auto* result = panel.get();
  YGNodeStyleSetWidth(panel->GetYogaNode(), width);
  YGNodeStyleSetHeight(panel->GetYogaNode(), height);
  YGNodeStyleSetFlexGrow(panel->GetYogaNode(), 0.0f);
  YGNodeStyleSetFlexShrink(panel->GetYogaNode(), 0.0f);
  // 交互坐标测试关闭主题外边距，使边界和输入坐标保持直观的一一对应。
  YGNodeStyleSetMargin(panel->GetYogaNode(), YGEdgeAll, 0.0f);
  layout.Root->AddChild(std::move(panel));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  return result;
}

} // namespace

TEST(PanelNodeTest, KeepsTitleAndContentAsInternalChildren) {
  PanelNode panel;
  ASSERT_NE(panel.TitleNode, nullptr);
  ASSERT_NE(panel.ScrollViewportNode, nullptr);
  ASSERT_NE(panel.ContentNode, nullptr);
  ASSERT_NE(panel.VerticalScrollBarNode, nullptr);
  ASSERT_NE(panel.VerticalScrollThumbNode, nullptr);
  EXPECT_EQ(panel.GetChildSize(), 3U);
  EXPECT_EQ(panel.ContentHost(), panel.ContentNode);
  EXPECT_EQ(panel.ContentNode->Parent, panel.ScrollViewportNode);

  auto content = std::make_unique<RectNode>();
  auto* contentObserver = content.get();
  panel.ContentHost()->AddChild(std::move(content));
  EXPECT_EQ(panel.ContentNode->GetChild(0), contentObserver);
  EXPECT_EQ(panel.TitleNode->GetChildSize(), 0U);
}

TEST(PanelNodeTest, ImplementsPropertyCapabilityInterfaces) {
  PanelNode panel;
  EXPECT_NE(dynamic_cast<IProperty*>(&panel), nullptr);
  EXPECT_NE(dynamic_cast<IDraggable*>(&panel), nullptr);
  EXPECT_NE(dynamic_cast<IResizable*>(&panel), nullptr);
  EXPECT_NE(dynamic_cast<IScrollable*>(&panel), nullptr);
}

TEST(PanelNodeTest, CapabilitySettersDispatchThroughInterfaces) {
  PanelNode panel;

  auto* draggable = dynamic_cast<IDraggable*>(&panel);
  DragProperty drag = draggable->GetDragProperties();
  drag.Enabled = false;
  draggable->SetDragProperties(drag);
  EXPECT_FALSE(panel.GetDragProperties().Enabled);

  auto* resizable = dynamic_cast<IResizable*>(&panel);
  ResizeProperty resize = resizable->GetResizeProperties();
  resize.MinWidth = 320.0f;
  resizable->SetResizeProperties(resize);
  // 虚接口必须分派回 Panel，确保能力状态和 Yoga 约束不会分离。
  EXPECT_FLOAT_EQ(YGNodeStyleGetMinWidth(panel.GetYogaNode()).value, 320.0f);

  auto* scrollable = dynamic_cast<IScrollable*>(&panel);
  ScrollProperty scroll = scrollable->GetScrollProperties();
  scroll.Enabled = false;
  scrollable->SetScrollProperties(scroll);
  EXPECT_FALSE(panel.GetScrollProperties().Enabled);
  EXPECT_EQ(YGNodeStyleGetOverflow(panel.ScrollViewportNode->GetYogaNode()),
            YGOverflowVisible);
}

TEST(PanelNodeTest, ScrollsOverflowAndShowsVerticalThumb) {
  Layout layout(nullptr);
  auto* panel = AddPanel(layout, 300.0f, 200.0f);
  for (int i = 0; i < 8; ++i) {
    auto item = std::make_unique<RectNode>();
    YGNodeStyleSetHeight(item->GetYogaNode(), 48.0f);
    YGNodeStyleSetFlexGrow(item->GetYogaNode(), 0.0f);
    YGNodeStyleSetFlexShrink(item->GetYogaNode(), 0.0f);
    YGNodeStyleSetMargin(item->GetYogaNode(), YGEdgeAll, 2.0f);
    panel->ContentHost()->AddChild(std::move(item));
  }
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  layout.Calculate(800.0f, 600.0f);

  ASSERT_GT(panel->GetMaximumScrollOffsetY(), 0.0f);
  EXPECT_TRUE(panel->VerticalScrollBarNode->IsVisible());
  const float originalItemY = panel->ContentNode->GetChild(1)->GetLayoutY();
  MouseWheelArgs wheel;
  wheel.X = 100;
  wheel.Y = 100;
  wheel.Delta = -WHEEL_DELTA;
  EXPECT_TRUE(layout.OnMouseWheel(wheel));
  EXPECT_FLOAT_EQ(panel->GetScrollOffsetY(),
                  panel->GetScrollProperties().WheelStep);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_LT(panel->ContentNode->GetChild(1)->GetLayoutY(), originalItemY);
  EXPECT_LT(panel->VerticalScrollThumbNode->GetLayoutHeight(),
            panel->VerticalScrollBarNode->GetLayoutHeight());
}

TEST(PanelNodeTest, AppliesTitleAndTitleHeight) {
  PanelNode panel;
  EXPECT_TRUE(panel.SetProperty("Title", "Inspector"));
  EXPECT_TRUE(panel.SetProperty("TitleHeight", "40"));
  EXPECT_EQ(panel.Title, "Inspector");

  YGNodeStyleSetWidth(panel.GetYogaNode(), 300.0f);
  YGNodeStyleSetHeight(panel.GetYogaNode(), 200.0f);
  YGNodeCalculateLayout(panel.GetYogaNode(), 300.0f, 200.0f, YGDirectionLTR);
  EXPECT_FLOAT_EQ(YGNodeLayoutGetHeight(panel.TitleNode->GetYogaNode()), 40.0f);
}

TEST(PanelNodeTest, DragsFromTitleAndKeepsYogaPosition) {
  Layout layout(nullptr);
  auto* panel = AddPanel(layout);
  const float originalWidth = panel->GetLayoutWidth();
  const float originalHeight = panel->GetLayoutHeight();

  EXPECT_TRUE(layout.OnMouseDown(MouseArgs(50, 16)));
  EXPECT_TRUE(layout.OnMouseDrag(MouseArgs(80, 46, 30, 30)));
  EXPECT_TRUE(layout.OnMouseUp(MouseArgs(80, 46)));
  layout.Calculate(800.0f, 600.0f);

  EXPECT_EQ(YGNodeStyleGetPositionType(panel->GetYogaNode()),
            YGPositionTypeAbsolute);
  EXPECT_FLOAT_EQ(panel->GetLayoutX(), 30.0f);
  EXPECT_FLOAT_EQ(panel->GetLayoutY(), 30.0f);
  EXPECT_FLOAT_EQ(panel->GetLayoutWidth(), originalWidth);
  EXPECT_FLOAT_EQ(panel->GetLayoutHeight(), originalHeight);
}

TEST(PanelNodeTest, BorderClickDoesNotMovePanelWithThemeMargin) {
  Layout layout(nullptr);
  auto panel = std::make_unique<PanelNode>();
  auto* panelObserver = panel.get();
  YGNodeStyleSetWidth(panel->GetYogaNode(), 300.0f);
  YGNodeStyleSetHeight(panel->GetYogaNode(), 200.0f);
  YGNodeStyleSetFlexGrow(panel->GetYogaNode(), 0.0f);
  YGNodeStyleSetFlexShrink(panel->GetYogaNode(), 0.0f);
  layout.Root->AddChild(std::move(panel));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  const float originalX = panelObserver->GetLayoutX();
  const float originalY = panelObserver->GetLayoutY();

  ASSERT_TRUE(layout.OnMouseDown(
      MouseArgs(static_cast<int>(originalX), 100)));
  ASSERT_TRUE(layout.OnMouseUp(MouseArgs(static_cast<int>(originalX), 100)));
  layout.Calculate(800.0f, 600.0f);

  EXPECT_EQ(YGNodeStyleGetPositionType(panelObserver->GetYogaNode()),
            YGPositionTypeRelative);
  EXPECT_FALSE(panelObserver->HasInteractiveGeometry());
  EXPECT_FLOAT_EQ(panelObserver->GetLayoutX(), originalX);
  EXPECT_FLOAT_EQ(panelObserver->GetLayoutY(), originalY);
}

TEST(PanelNodeTest, ResizesFromCornerAndHonorsMinimumSize) {
  Layout layout(nullptr);
  auto* panel = AddPanel(layout);

  EXPECT_TRUE(layout.OnMouseDown(MouseArgs(299, 199)));
  EXPECT_TRUE(layout.OnMouseDrag(MouseArgs(349, 239, 50, 40)));
  EXPECT_TRUE(layout.OnMouseUp(MouseArgs(349, 239)));
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(panel->GetLayoutWidth(), 350.0f);
  EXPECT_FLOAT_EQ(panel->GetLayoutHeight(), 240.0f);

  EXPECT_TRUE(layout.OnMouseDown(MouseArgs(1, 100)));
  EXPECT_TRUE(layout.OnMouseDrag(MouseArgs(471, 100, 470, 0)));
  EXPECT_TRUE(layout.OnMouseUp(MouseArgs(471, 100)));
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(panel->GetLayoutWidth(),
                  panel->GetResizeProperties().MinWidth);
  EXPECT_FLOAT_EQ(panel->GetLayoutX(), 110.0f);
}

TEST(PanelNodeTest, SelectsResizeCursorForEveryBorderDirection) {
  Layout layout(nullptr);
  AddPanel(layout);

  EXPECT_EQ(layout.GetMouseCursor(1, 100), MouseCursor::SizeHorizontal);
  EXPECT_EQ(layout.GetMouseCursor(150, 1), MouseCursor::SizeVertical);
  EXPECT_EQ(layout.GetMouseCursor(1, 1),
            MouseCursor::SizeDiagonalNorthwestSoutheast);
  EXPECT_EQ(layout.GetMouseCursor(299, 1),
            MouseCursor::SizeDiagonalNortheastSouthwest);
  EXPECT_EQ(layout.GetMouseCursor(150, 100), MouseCursor::Arrow);

  ASSERT_TRUE(layout.OnMouseDown(MouseArgs(1, 100)));
  EXPECT_EQ(layout.GetMouseCursor(500, 100), MouseCursor::SizeHorizontal);
  ASSERT_TRUE(layout.OnMouseUp(MouseArgs(500, 100)));
}

TEST(PanelNodeTest, ExposesSeparatedDefaultBehaviorProperties) {
  PanelNode panel;
  EXPECT_TRUE(panel.GetDragProperties().Enabled);
  EXPECT_EQ(panel.GetDragProperties().Region, DragRegion::TitleBar);
  EXPECT_TRUE(panel.GetResizeProperties().Enabled);
  EXPECT_TRUE(panel.GetScrollProperties().Enabled);
  EXPECT_FALSE(panel.GetScrollProperties().Horizontal);
  EXPECT_TRUE(panel.GetScrollProperties().Vertical);
  EXPECT_EQ(panel.GetScrollProperties().HorizontalScrollBar,
            ScrollBarVisibility::Hidden);
  EXPECT_EQ(panel.GetScrollProperties().VerticalScrollBar,
            ScrollBarVisibility::Auto);

  EXPECT_TRUE(panel.SetProperty("DragRegion", "Anywhere"));
  EXPECT_TRUE(panel.SetProperty("Scrollable", "false"));
  EXPECT_TRUE(panel.SetProperty("ShowHorizontalScrollBar", "true"));
  EXPECT_EQ(panel.GetDragProperties().Region, DragRegion::Anywhere);
  EXPECT_FALSE(panel.GetScrollProperties().Enabled);
  EXPECT_EQ(panel.GetScrollProperties().HorizontalScrollBar,
            ScrollBarVisibility::Visible);
}

TEST(PanelNodeTest, AllowsDraggingFromContentWhenConfigured) {
  Layout layout(nullptr);
  auto* panel = AddPanel(layout);
  ASSERT_TRUE(panel->SetProperty("DragRegion", "Anywhere"));

  ASSERT_TRUE(layout.OnMouseDown(MouseArgs(150, 100)));
  ASSERT_TRUE(layout.OnMouseDrag(MouseArgs(170, 120, 20, 20)));
  ASSERT_TRUE(layout.OnMouseUp(MouseArgs(170, 120)));
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(panel->GetLayoutX(), 20.0f);
  EXPECT_FLOAT_EQ(panel->GetLayoutY(), 20.0f);
}

TEST(PanelNodeTest, KeepsInteractiveSizeAcrossImmediateDeclarations) {
  Layout layout(nullptr);
  ImmediateUI ui(layout);
  UIStyle style;
  style.Width = 300.0f;
  style.Height = 200.0f;
  style.FlexGrow = 0.0f;
  style.FlexShrink = 0.0f;
  style.Margin = 0.0f;

  ui.BeginFrame();
  ASSERT_TRUE(ui.BeginPanel("panel", "Panel", style));
  ui.EndPanel();
  ASSERT_TRUE(ui.EndFrame());
  layout.Calculate(800.0f, 600.0f);
  ASSERT_TRUE(layout.OnMouseDown(MouseArgs(299, 199)));
  ASSERT_TRUE(layout.OnMouseDrag(MouseArgs(349, 239, 50, 40)));
  ASSERT_TRUE(layout.OnMouseUp(MouseArgs(349, 239)));

  // 重放原始声明只表达默认尺寸，不应覆盖用户已经提交的运行时几何。
  ui.BeginFrame();
  ASSERT_TRUE(ui.BeginPanel("panel", "Panel", style));
  ui.EndPanel();
  ASSERT_TRUE(ui.EndFrame());
  layout.Calculate(800.0f, 600.0f);
  auto* panel = dynamic_cast<PanelNode*>(layout.Find("panel"));
  ASSERT_NE(panel, nullptr);
  EXPECT_FLOAT_EQ(panel->GetLayoutWidth(), 350.0f);
  EXPECT_FLOAT_EQ(panel->GetLayoutHeight(), 240.0f);
}
} // namespace z8::ui
