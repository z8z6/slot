#include "UI/Layout/PanelNode.h"
#include "UI/Declarative/ImmediateUI.h"
#include "UI/Layout/Layout.h"

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

PanelNode *AddPanel(Layout &layout, float width = 300.0f,
                    float height = 200.0f) {
  auto panel = std::make_unique<PanelNode>();
  auto *result = panel.get();
  YGNodeStyleSetWidth(panel->Node, width);
  YGNodeStyleSetHeight(panel->Node, height);
  YGNodeStyleSetFlexGrow(panel->Node, 0.0f);
  YGNodeStyleSetFlexShrink(panel->Node, 0.0f);
  // 交互坐标测试关闭主题外边距，使边界和输入坐标保持直观的一一对应。
  YGNodeStyleSetMargin(panel->Node, YGEdgeAll, 0.0f);
  // 通用交互测试使用浮动模式，避免根 DockSpace 有意覆盖显式测试几何。
  auto *dock = result->GetBehavior<DockBehavior>();
  dock->Properties.Enabled = false;
  dock->Properties.Placement = DockPlacement::Floating;
  layout.Root->AddChild(std::move(panel));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  return result;
}

} // namespace

TEST(PanelNodeTest, KeepsTitleAndContentAsInternalChildren) {
  PanelNode panel;
  ASSERT_NE(panel.TitleNode, nullptr);
  ASSERT_NE(panel.ScrollAreaNode, nullptr);
  ASSERT_NE(panel.ScrollAreaNode->ViewportNode, nullptr);
  ASSERT_NE(panel.ScrollAreaNode->ContentNode, nullptr);
  ASSERT_NE(panel.ScrollAreaNode->VerticalScrollBarNode, nullptr);
  ASSERT_NE(panel.ScrollAreaNode->VerticalScrollThumbNode, nullptr);
  EXPECT_EQ(panel.Children.size(), 2U);
  EXPECT_EQ(panel.ContentHost(), panel.ScrollAreaNode->ContentNode);
  EXPECT_EQ(panel.ScrollAreaNode->ContentNode->Parent,
            panel.ScrollAreaNode->ViewportNode);

  auto content = std::make_unique<RectNode>();
  auto *contentObserver = content.get();
  panel.ContentHost()->AddChild(std::move(content));
  ASSERT_EQ(panel.ScrollAreaNode->ContentNode->Children.size(), 1U);
  EXPECT_EQ(panel.ScrollAreaNode->ContentNode->Children[0].get(),
            contentObserver);
  EXPECT_EQ(panel.TitleNode->Children.size(), 0U);
}

TEST(PanelNodeTest, ComposesIndependentBehaviors) {
  PanelNode panel;
  EXPECT_NE(dynamic_cast<IProperty *>(&panel), nullptr);
  EXPECT_NE(panel.GetBehavior<DragBehavior>(), nullptr);
  EXPECT_NE(panel.GetBehavior<ResizeBehavior>(), nullptr);
  EXPECT_NE(panel.ScrollAreaNode->GetScrollBehavior(), nullptr);
  EXPECT_NE(panel.GetBehavior<DockBehavior>(), nullptr);
}

TEST(PanelNodeTest, BehaviorConfigurationMaintainsLayoutInvariants) {
  PanelNode panel;

  auto *drag = panel.GetBehavior<DragBehavior>();
  drag->Properties.Enabled = false;
  EXPECT_FALSE(drag->Properties.Enabled);

  auto *resizable = panel.GetBehavior<ResizeBehavior>();
  ResizeProperty resize = resizable->Properties;
  resize.MinWidth = 320.0f;
  resizable->SetProperties(resize);
  // 行为 setter 必须同时更新 Yoga，确保配置与布局约束不会分离。
  EXPECT_FLOAT_EQ(YGNodeStyleGetMinWidth(panel.Node).value, 320.0f);

  auto *scrollable = panel.ScrollAreaNode->GetScrollBehavior();
  ScrollProperty scroll = scrollable->Properties;
  scroll.Enabled = false;
  scrollable->SetProperties(scroll);
  EXPECT_FALSE(scrollable->Properties.Enabled);
  EXPECT_EQ(YGNodeStyleGetOverflow(panel.ScrollAreaNode->ViewportNode->Node),
            YGOverflowVisible);
}

TEST(PanelNodeTest, ScrollsOverflowAndShowsVerticalThumb) {
  Layout layout;
  auto *panel = AddPanel(layout, 300.0f, 200.0f);
  for (int i = 0; i < 8; ++i) {
    auto item = std::make_unique<RectNode>();
    YGNodeStyleSetHeight(item->Node, 48.0f);
    YGNodeStyleSetFlexGrow(item->Node, 0.0f);
    YGNodeStyleSetFlexShrink(item->Node, 0.0f);
    YGNodeStyleSetMargin(item->Node, YGEdgeAll, 2.0f);
    panel->ContentHost()->AddChild(std::move(item));
  }
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  layout.Calculate(800.0f, 600.0f);

  auto *scroll = panel->ScrollAreaNode->GetScrollBehavior();
  ASSERT_GT(scroll->GetMaximumOffsetY(), 0.0f);
  EXPECT_TRUE(panel->ScrollAreaNode->VerticalScrollBarNode->Visible);
  const float originalItemY =
      panel->ScrollAreaNode->ContentNode->Children[1]->Top;
  MouseWheelArgs wheel;
  wheel.X = 100;
  wheel.Y = 100;
  wheel.Delta = -WHEEL_DELTA;
  EXPECT_NE(layout.OnMouseWheel(wheel), EventReply::Ignored);
  EXPECT_FLOAT_EQ(scroll->GetOffsetY(), scroll->Properties.WheelStep);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_LT(panel->ScrollAreaNode->ContentNode->Children[1]->Top,
            originalItemY);
  EXPECT_LT(panel->ScrollAreaNode->VerticalScrollThumbNode->Height,
            panel->ScrollAreaNode->VerticalScrollBarNode->Height);
}

TEST(PanelNodeTest, AppliesTitleAndTitleHeight) {
  PanelNode panel;
  EXPECT_TRUE(panel.SetProperty("Title", "Inspector"));
  EXPECT_TRUE(panel.SetProperty("TitleHeight", "40"));
  EXPECT_EQ(panel.TitleNode->Text, "Inspector");

  YGNodeStyleSetWidth(panel.Node, 300.0f);
  YGNodeStyleSetHeight(panel.Node, 200.0f);
  YGNodeCalculateLayout(panel.Node, 300.0f, 200.0f, YGDirectionLTR);
  EXPECT_FLOAT_EQ(YGNodeLayoutGetHeight(panel.TitleNode->Node), 40.0f);
}

TEST(PanelNodeTest, DragsFromTitleAndKeepsYogaPosition) {
  Layout layout;
  auto *panel = AddPanel(layout);
  const float originalWidth = panel->Width;
  const float originalHeight = panel->Height;

  EXPECT_NE(layout.OnMouseDown(MouseArgs(50, 16)), EventReply::Ignored);
  EXPECT_NE(layout.OnMouseDrag(MouseArgs(80, 46, 30, 30)),
            EventReply::Ignored);
  EXPECT_NE(layout.OnMouseUp(MouseArgs(80, 46)), EventReply::Ignored);
  layout.Calculate(800.0f, 600.0f);

  EXPECT_EQ(YGNodeStyleGetPositionType(panel->Node), YGPositionTypeAbsolute);
  EXPECT_FLOAT_EQ(panel->Left, 30.0f);
  EXPECT_FLOAT_EQ(panel->Top, 30.0f);
  EXPECT_FLOAT_EQ(panel->Width, originalWidth);
  EXPECT_FLOAT_EQ(panel->Height, originalHeight);
}

TEST(PanelNodeTest, BorderClickDoesNotMovePanelWithThemeMargin) {
  Layout layout;
  auto panel = std::make_unique<PanelNode>();
  auto *panelObserver = panel.get();
  YGNodeStyleSetWidth(panel->Node, 300.0f);
  YGNodeStyleSetHeight(panel->Node, 200.0f);
  YGNodeStyleSetFlexGrow(panel->Node, 0.0f);
  YGNodeStyleSetFlexShrink(panel->Node, 0.0f);
  auto *dock = panelObserver->GetBehavior<DockBehavior>();
  dock->Properties.Enabled = false;
  dock->Properties.Placement = DockPlacement::Floating;
  layout.Root->AddChild(std::move(panel));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  const float originalX = panelObserver->Left;
  const float originalY = panelObserver->Top;

  ASSERT_NE(layout.OnMouseDown(MouseArgs(static_cast<int>(originalX), 100)),
            EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(MouseArgs(static_cast<int>(originalX), 100)),
            EventReply::Ignored);
  layout.Calculate(800.0f, 600.0f);

  EXPECT_EQ(YGNodeStyleGetPositionType(panelObserver->Node),
            YGPositionTypeRelative);
  EXPECT_FALSE(panelObserver->HasInteractiveGeometry());
  EXPECT_FLOAT_EQ(panelObserver->Left, originalX);
  EXPECT_FLOAT_EQ(panelObserver->Top, originalY);
}

TEST(PanelNodeTest, ResizesFromCornerAndHonorsMinimumSize) {
  Layout layout;
  auto *panel = AddPanel(layout);

  EXPECT_NE(layout.OnMouseDown(MouseArgs(299, 199)), EventReply::Ignored);
  EXPECT_NE(layout.OnMouseDrag(MouseArgs(349, 239, 50, 40)),
            EventReply::Ignored);
  EXPECT_NE(layout.OnMouseUp(MouseArgs(349, 239)), EventReply::Ignored);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(panel->Width, 350.0f);
  EXPECT_FLOAT_EQ(panel->Height, 240.0f);

  EXPECT_NE(layout.OnMouseDown(MouseArgs(1, 100)), EventReply::Ignored);
  EXPECT_NE(layout.OnMouseDrag(MouseArgs(471, 100, 470, 0)),
            EventReply::Ignored);
  EXPECT_NE(layout.OnMouseUp(MouseArgs(471, 100)), EventReply::Ignored);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(panel->Width,
                  panel->GetBehavior<ResizeBehavior>()->Properties.MinWidth);
  EXPECT_FLOAT_EQ(panel->Left, 110.0f);
}

TEST(PanelNodeTest, SelectsResizeCursorForEveryBorderDirection) {
  Layout layout;
  AddPanel(layout);

  EXPECT_EQ(layout.GetMouseCursor(MouseArgs(1, 100)),
            MouseCursor::SizeHorizontal);
  EXPECT_EQ(layout.GetMouseCursor(MouseArgs(150, 1)),
            MouseCursor::SizeVertical);
  EXPECT_EQ(layout.GetMouseCursor(MouseArgs(1, 1)),
            MouseCursor::SizeDiagonalNorthwestSoutheast);
  EXPECT_EQ(layout.GetMouseCursor(MouseArgs(299, 1)),
            MouseCursor::SizeDiagonalNortheastSouthwest);
  EXPECT_EQ(layout.GetMouseCursor(MouseArgs(150, 100)), MouseCursor::Arrow);

  ASSERT_NE(layout.OnMouseDown(MouseArgs(1, 100)), EventReply::Ignored);
  EXPECT_EQ(layout.GetMouseCursor(MouseArgs(500, 100)),
            MouseCursor::SizeHorizontal);
  ASSERT_NE(layout.OnMouseUp(MouseArgs(500, 100)), EventReply::Ignored);
}

TEST(PanelNodeTest, ExposesSeparatedDefaultBehaviorProperties) {
  PanelNode panel;
  auto *drag = panel.GetBehavior<DragBehavior>();
  auto *resize = panel.GetBehavior<ResizeBehavior>();
  auto *scroll = panel.ScrollAreaNode->GetScrollBehavior();
  EXPECT_TRUE(drag->Properties.Enabled);
  EXPECT_EQ(drag->Properties.Region, DragRegion::TitleBar);
  EXPECT_TRUE(resize->Properties.Enabled);
  EXPECT_TRUE(scroll->Properties.Enabled);
  EXPECT_FALSE(scroll->Properties.Horizontal);
  EXPECT_TRUE(scroll->Properties.Vertical);
  EXPECT_EQ(scroll->Properties.HorizontalScrollBar,
            ScrollBarVisibility::Hidden);
  EXPECT_EQ(scroll->Properties.VerticalScrollBar,
            ScrollBarVisibility::Auto);

  EXPECT_TRUE(panel.SetProperty("DragRegion", "Anywhere"));
  EXPECT_TRUE(panel.SetProperty("Scrollable", "false"));
  EXPECT_TRUE(panel.SetProperty("ShowHorizontalScrollBar", "true"));
  EXPECT_EQ(drag->Properties.Region, DragRegion::Anywhere);
  EXPECT_FALSE(scroll->Properties.Enabled);
  EXPECT_EQ(scroll->Properties.HorizontalScrollBar,
            ScrollBarVisibility::Visible);
}

TEST(PanelNodeTest, AllowsDraggingFromContentWhenConfigured) {
  Layout layout;
  auto *panel = AddPanel(layout);
  ASSERT_TRUE(panel->SetProperty("DragRegion", "Anywhere"));

  ASSERT_NE(layout.OnMouseDown(MouseArgs(150, 100)), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseDrag(MouseArgs(170, 120, 20, 20)),
            EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(MouseArgs(170, 120)), EventReply::Ignored);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(panel->Left, 20.0f);
  EXPECT_FLOAT_EQ(panel->Top, 20.0f);
}

TEST(PanelNodeTest, AutomaticallyTilesMultiplePanelsInDockSpace) {
  Layout layout;
  auto first = std::make_unique<PanelNode>();
  auto second = std::make_unique<PanelNode>();
  auto *firstObserver = first.get();
  auto *secondObserver = second.get();
  layout.Root->AddChild(std::move(first));
  layout.Root->AddChild(std::move(second));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  EXPECT_FLOAT_EQ(firstObserver->Left, 0.0f);
  EXPECT_FLOAT_EQ(firstObserver->Width, 400.0f);
  EXPECT_FLOAT_EQ(secondObserver->Left, 400.0f);
  EXPECT_FLOAT_EQ(secondObserver->Width, 400.0f);
  EXPECT_FLOAT_EQ(firstObserver->Height, 600.0f);
}

TEST(PanelNodeTest, DocksPanelAtNearestEdgeAfterDragging) {
  Layout layout;
  auto first = std::make_unique<PanelNode>();
  auto second = std::make_unique<PanelNode>();
  auto *firstObserver = first.get();
  auto *secondObserver = second.get();
  layout.Root->AddChild(std::move(first));
  layout.Root->AddChild(std::move(second));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  ASSERT_NE(layout.OnMouseDown(MouseArgs(100, 16)), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseDrag(MouseArgs(790, 100, 690, 84)),
            EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(MouseArgs(790, 100)), EventReply::Ignored);
  EXPECT_EQ(firstObserver->GetBehavior<DockBehavior>()->Properties.Placement,
            DockPlacement::Right);

  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(firstObserver->Left, 500.0f);
  EXPECT_FLOAT_EQ(firstObserver->Width, 300.0f);
  EXPECT_FLOAT_EQ(secondObserver->Left, 0.0f);
  EXPECT_FLOAT_EQ(secondObserver->Width, 500.0f);
}

TEST(PanelNodeTest, KeepsInteractiveSizeAcrossImmediateDeclarations) {
  Layout layout;
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
  auto *initialPanel = dynamic_cast<PanelNode *>(layout.Find("panel"));
  ASSERT_NE(initialPanel, nullptr);
  initialPanel->GetBehavior<DockBehavior>()->Properties.Placement =
      DockPlacement::Floating;
  layout.Calculate(800.0f, 600.0f);
  ASSERT_NE(layout.OnMouseDown(MouseArgs(299, 199)), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseDrag(MouseArgs(349, 239, 50, 40)),
            EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(MouseArgs(349, 239)), EventReply::Ignored);

  // 重放原始声明只表达默认尺寸，不应覆盖用户已经提交的运行时几何。
  ui.BeginFrame();
  ASSERT_TRUE(ui.BeginPanel("panel", "Panel", style));
  ui.EndPanel();
  ASSERT_TRUE(ui.EndFrame());
  layout.Calculate(800.0f, 600.0f);
  auto *panel = dynamic_cast<PanelNode *>(layout.Find("panel"));
  ASSERT_NE(panel, nullptr);
  EXPECT_FLOAT_EQ(panel->Width, 350.0f);
  EXPECT_FLOAT_EQ(panel->Height, 240.0f);
}
} // namespace z8::ui
