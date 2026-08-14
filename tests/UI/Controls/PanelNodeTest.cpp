#include "UI/Layout/PanelNode.h"
#include "Object/UIObject/UIObject.h"
#include "UI/Declarative/ImmediateUI.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Style/Theme.h"

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
  panel->Style.Width = width;
  panel->Style.Height = height;
  panel->Style.FlexGrow = 0.0f;
  panel->Style.FlexShrink = 0.0f;
  // 交互坐标测试关闭主题外边距，使边界和输入坐标保持直观的一一对应。
  panel->Style.Margin = 0.0f;
  // 通用行为测试关闭 Dock，使显式测试几何不被根 DockSpace 覆盖。
  auto *dock = result->GetBehavior<DockBehavior>();
  dock->Properties.Enabled = false;
  layout.Root->AddChild(std::move(panel));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  return result;
}

} // namespace

TEST(PanelNodeTest, KeepsTitleAndContentAsInternalChildren) {
  PanelNode panel;
  ASSERT_NE(panel.TitleBarNode, nullptr);
  ASSERT_NE(panel.TitleNode, nullptr);
  ASSERT_NE(panel.ScrollAreaNode, nullptr);
  ASSERT_NE(panel.ScrollAreaNode->ViewportNode, nullptr);
  ASSERT_NE(panel.ScrollAreaNode->ContentNode, nullptr);
  ASSERT_NE(panel.ScrollAreaNode->VerticalScrollBarNode, nullptr);
  ASSERT_NE(panel.ScrollAreaNode->VerticalScrollThumbNode, nullptr);
  EXPECT_EQ(panel.Children.size(), 2U);
  EXPECT_EQ(panel.Children[0].get(), panel.TitleBarNode);
  EXPECT_EQ(panel.TitleNode->Parent, panel.TitleBarNode);
  EXPECT_EQ(panel.ContentHost(), panel.ScrollAreaNode->ContentNode);
  EXPECT_EQ(panel.ScrollAreaNode->ContentNode->Parent,
            panel.ScrollAreaNode->ViewportNode);
  ASSERT_TRUE(
      panel.ScrollAreaNode->VerticalScrollBarNode->Style.Right.has_value());
  EXPECT_FLOAT_EQ(
      panel.ScrollAreaNode->VerticalScrollBarNode->Style.Right.value(), 0.0f);

  auto content = std::make_unique<RectNode>();
  auto *contentObserver = content.get();
  panel.ContentHost()->AddChild(std::move(content));
  ASSERT_EQ(panel.ScrollAreaNode->ContentNode->Children.size(), 1U);
  EXPECT_EQ(panel.ScrollAreaNode->ContentNode->Children[0].get(),
            contentObserver);
  EXPECT_EQ(panel.TitleNode->Children.size(), 0U);
}

TEST(PanelNodeTest, UsesVisibleDefaultBorder) {
  PanelNode panel;
  const auto &style = Theme::Default().Panel;

  EXPECT_FLOAT_EQ(panel.UO->GetBorderWidth(), style.BorderWidth);
  EXPECT_FLOAT_EQ(panel.UO->GetBorderColor().x, style.BorderColor.x);
  EXPECT_FLOAT_EQ(panel.TitleBarNode->UO->GetColor().x, style.TitleColor.x);
  EXPECT_NE(panel.TitleBarNode->UO->GetColor().x, panel.UO->GetColor().x);
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
  // 行为 setter 必须同时更新布局样式，确保配置与布局约束不会分离。
  EXPECT_FLOAT_EQ(panel.Style.MinWidth, 320.0f);

  auto *scrollable = panel.ScrollAreaNode->GetScrollBehavior();
  ScrollProperty scroll = scrollable->Properties;
  scroll.Enabled = false;
  scrollable->SetProperties(scroll);
  EXPECT_FALSE(scrollable->Properties.Enabled);
  EXPECT_FALSE(panel.ScrollAreaNode->ViewportNode->ClipChildren);
}

TEST(PanelNodeTest, ScrollsOverflowAndShowsVerticalThumb) {
  Layout layout;
  auto *panel = AddPanel(layout, 300.0f, 200.0f);
  for (int i = 0; i < 8; ++i) {
    auto item = std::make_unique<RectNode>();
    item->Style.Height = 48.0f;
    item->Style.FlexGrow = 0.0f;
    item->Style.FlexShrink = 0.0f;
    item->Style.Margin = 2.0f;
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
  EXPECT_FLOAT_EQ(panel.TitleBarNode->Style.Height.value_or(0.0f), 40.0f);

  Layout layout;
  auto ownedPanel = std::make_unique<PanelNode>();
  ownedPanel->SetProperty("TitleHeight", "40");
  auto *observer = ownedPanel.get();
  ownedPanel->Style.Width = 300.0f;
  ownedPanel->Style.Height = 200.0f;
  layout.Root->AddChild(std::move(ownedPanel));
  layout.Calculate(300.0f, 200.0f);
  EXPECT_FLOAT_EQ(observer->TitleNode->Computed.Height, 40.0f);
}

TEST(PanelNodeTest, DragsFromTitleAndKeepsAbsolutePosition) {
  Layout layout;
  auto *panel = AddPanel(layout);
  const float originalWidth = panel->Width;
  const float originalHeight = panel->Height;

  EXPECT_NE(layout.OnMouseDown(MouseArgs(50, 16)), EventReply::Ignored);
  EXPECT_NE(layout.OnMouseDrag(MouseArgs(80, 46, 30, 30)), EventReply::Ignored);
  EXPECT_NE(layout.OnMouseUp(MouseArgs(80, 46)), EventReply::Ignored);
  layout.Calculate(800.0f, 600.0f);

  EXPECT_EQ(panel->Style.Position, PositionType::Absolute);
  EXPECT_FLOAT_EQ(panel->Left, 30.0f);
  EXPECT_FLOAT_EQ(panel->Top, 30.0f);
  EXPECT_FLOAT_EQ(panel->Width, originalWidth);
  EXPECT_FLOAT_EQ(panel->Height, originalHeight);
}

TEST(PanelNodeTest, BorderClickDoesNotMovePanelWithThemeMargin) {
  Layout layout;
  auto panel = std::make_unique<PanelNode>();
  auto *panelObserver = panel.get();
  panel->Style.Width = 300.0f;
  panel->Style.Height = 200.0f;
  panel->Style.FlexGrow = 0.0f;
  panel->Style.FlexShrink = 0.0f;
  auto *dock = panelObserver->GetBehavior<DockBehavior>();
  dock->Properties.Enabled = false;
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

  EXPECT_EQ(panelObserver->Style.Position, PositionType::Relative);
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
  EXPECT_EQ(scroll->Properties.VerticalScrollBar, ScrollBarVisibility::Auto);

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

  ASSERT_NE(firstObserver->Group, nullptr);
  ASSERT_NE(secondObserver->Group, nullptr);
  EXPECT_FLOAT_EQ(firstObserver->Group->Left, 0.0f);
  EXPECT_FLOAT_EQ(firstObserver->Group->Width, 400.0f);
  EXPECT_FLOAT_EQ(secondObserver->Group->Left, 400.0f);
  EXPECT_FLOAT_EQ(secondObserver->Group->Width, 400.0f);
  EXPECT_FLOAT_EQ(firstObserver->Group->Height, 600.0f);
}

TEST(PanelNodeTest, ResizesSharedDockBoundaryPersistently) {
  Layout layout;
  auto first = std::make_unique<PanelNode>();
  auto second = std::make_unique<PanelNode>();
  auto *firstObserver = first.get();
  auto *secondObserver = second.get();
  layout.Root->AddChild(std::move(first));
  layout.Root->AddChild(std::move(second));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  ASSERT_NE(layout.OnMouseDown(MouseArgs(400, 300)), EventReply::Ignored);
  ASSERT_NE(layout.CapturedSplitter, 0U);
  ASSERT_NE(layout.OnMouseDrag(MouseArgs(500, 300, 100, 0)),
            EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(MouseArgs(500, 300)), EventReply::Ignored);

  // 连续两帧都必须保持新分隔位置，证明结果已进入 Dock 状态而非临时布局值。
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(firstObserver->Width, 500.0f);
  EXPECT_FLOAT_EQ(secondObserver->Left, 500.0f);
  EXPECT_FLOAT_EQ(secondObserver->Width, 300.0f);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(firstObserver->Width, 500.0f);
  EXPECT_FLOAT_EQ(secondObserver->Width, 300.0f);
}

TEST(PanelNodeTest, ResizesTopDockFromSharedLowerPanelEdge) {
  Layout layout;
  auto top = std::make_unique<PanelNode>();
  auto fill = std::make_unique<PanelNode>();
  auto *topObserver = top.get();
  auto *fillObserver = fill.get();
  auto *topDock = topObserver->GetBehavior<DockBehavior>();
  topDock->Properties.Placement = DockPlacement::Top;
  topDock->Properties.Extent = 200.0f;
  auto *fillDock = fillObserver->GetBehavior<DockBehavior>();
  fillDock->Properties.Placement = DockPlacement::Fill;
  layout.Root->AddChild(std::move(top));
  layout.Root->AddChild(std::move(fill));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  // 共享边界由 DockTree Splitter 捕获，不再启动任一 Panel 的 ResizeBehavior。
  ASSERT_NE(layout.OnMouseDown(MouseArgs(400, 200)), EventReply::Ignored);
  ASSERT_NE(layout.CapturedSplitter, 0U);
  ASSERT_NE(layout.OnMouseDrag(MouseArgs(400, 250, 0, 50)),
            EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(MouseArgs(400, 250)), EventReply::Ignored);
  layout.Calculate(800.0f, 600.0f);

  EXPECT_EQ(layout.Dock.Tree.Root->Axis, SplitAxis::Horizontal);
  EXPECT_FLOAT_EQ(topObserver->Group->Height, 250.0f);
  EXPECT_FLOAT_EQ(fillObserver->Group->Top, 250.0f);
  EXPECT_FLOAT_EQ(fillObserver->Group->Height, 350.0f);
}

TEST(PanelNodeTest, StopsDockDividerAtMinimumHeight) {
  Layout layout;
  auto top = std::make_unique<PanelNode>();
  auto fill = std::make_unique<PanelNode>();
  auto *topObserver = top.get();
  auto *fillObserver = fill.get();
  auto *topDock = topObserver->GetBehavior<DockBehavior>();
  topDock->Properties.Placement = DockPlacement::Top;
  topDock->Properties.Extent = 200.0f;
  fillObserver->GetBehavior<DockBehavior>()->Properties.Placement =
      DockPlacement::Fill;
  layout.Root->AddChild(std::move(top));
  layout.Root->AddChild(std::move(fill));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  const float minimum =
      topObserver->GetBehavior<ResizeBehavior>()->Properties.MinHeight;
  ASSERT_NE(layout.OnMouseDown(MouseArgs(400, 199)), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseDrag(MouseArgs(400, 0, 0, -300)),
            EventReply::Ignored);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(topObserver->Group->Height, minimum);
  EXPECT_FLOAT_EQ(topObserver->Height, topObserver->Group->PagesNode->Height);
  const float stoppedTop = fillObserver->Top;

  // 已到最小高度后继续同向移动不应再改变分隔线位置。
  ASSERT_NE(layout.OnMouseDrag(MouseArgs(400, -100, 0, -100)),
            EventReply::Ignored);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(topObserver->Group->Height, minimum);
  EXPECT_FLOAT_EQ(topObserver->Height, topObserver->Group->PagesNode->Height);
  EXPECT_FLOAT_EQ(fillObserver->Top, stoppedTop);
  ASSERT_NE(layout.OnMouseUp(MouseArgs(400, -100)), EventReply::Ignored);
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
  layout.Calculate(800.0f, 600.0f);
  EXPECT_EQ(layout.Dock.Tree.FindPanelLeaf(firstObserver)->Parent->Axis,
            SplitAxis::Vertical);
  EXPECT_FLOAT_EQ(firstObserver->Left, 400.0f);
  EXPECT_FLOAT_EQ(firstObserver->Width, 400.0f);
  EXPECT_FLOAT_EQ(secondObserver->Left, 0.0f);
  EXPECT_FLOAT_EQ(secondObserver->Width, 400.0f);
}

TEST(PanelNodeTest, CenterDropCreatesFloatingWindow) {
  Layout layout;
  auto first = std::make_unique<PanelNode>();
  auto second = std::make_unique<PanelNode>();
  auto *panel = first.get();
  auto *dock = panel->GetBehavior<DockBehavior>();
  dock->Properties.EdgeThreshold = 0.0f;
  layout.Root->AddChild(std::move(first));
  layout.Root->AddChild(std::move(second));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  ASSERT_NE(layout.OnMouseDown(MouseArgs(100, 16)), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseDrag(MouseArgs(600, 300, 500, 284)),
            EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(MouseArgs(600, 300)), EventReply::Ignored);
  layout.Calculate(800.0f, 600.0f);
  const auto *state = layout.Dock.GetState(*panel);
  ASSERT_NE(state, nullptr);
  EXPECT_EQ(state->Placement, PanelPlacement::Floating);
  EXPECT_EQ(layout.Dock.Tree.FindPanelLeaf(panel), nullptr);
}

TEST(PanelNodeTest, DocksPanelAgainstSiblingAndReflowsWorkspace) {
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
  ASSERT_NE(layout.OnMouseDrag(MouseArgs(790, 300, 690, 284)),
            EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(MouseArgs(790, 300)), EventReply::Ignored);
  layout.Calculate(800.0f, 600.0f);

  EXPECT_EQ(layout.Dock.Tree.FindPanelLeaf(firstObserver)->Parent->Axis,
            SplitAxis::Vertical);
  EXPECT_FLOAT_EQ(firstObserver->Width, secondObserver->Width);
  EXPECT_LE(secondObserver->Left + secondObserver->Width, firstObserver->Left);
}

TEST(PanelNodeTest, DocksPanelsOnBothHorizontalSides) {
  const auto verifySide = [](bool dockLeft) {
    Layout layout;
    auto moving = std::make_unique<PanelNode>();
    auto target = std::make_unique<PanelNode>();
    auto *movingObserver = moving.get();
    auto *targetObserver = target.get();
    layout.Root->AddChild(std::move(moving));
    layout.Root->AddChild(std::move(target));
    layout.RebuildIndex();
    layout.Calculate(800.0f, 600.0f);
    EXPECT_FLOAT_EQ(movingObserver->Width, targetObserver->Width);

    ASSERT_NE(layout.OnMouseDown(MouseArgs(100, 16)), EventReply::Ignored);
    const int dropX =
        static_cast<int>(targetObserver->Left +
                         targetObserver->Width * (dockLeft ? 0.2f : 0.8f));
    const int dropY =
        static_cast<int>(targetObserver->Top + targetObserver->Height * 0.5f);
    ASSERT_NE(
        layout.OnMouseDrag(MouseArgs(dropX, dropY, dropX - 100, dropY - 16)),
        EventReply::Ignored);
    ASSERT_NE(layout.OnMouseUp(MouseArgs(dropX, dropY)), EventReply::Ignored);
    const auto *movingLeaf = layout.Dock.Tree.FindPanelLeaf(movingObserver);
    ASSERT_NE(movingLeaf, nullptr);
    EXPECT_EQ(movingLeaf->Parent->Axis, SplitAxis::Vertical);

    layout.Calculate(800.0f, 600.0f);
    if (dockLeft)
      EXPECT_LE(movingObserver->Left + movingObserver->Width,
                targetObserver->Left);
    else
      EXPECT_GE(movingObserver->Left,
                targetObserver->Left + targetObserver->Width);
  };

  verifySide(true);
  verifySide(false);
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
  ASSERT_NE(initialPanel->Group, nullptr);
  initialPanel->Group->GetBehavior<DockBehavior>()->Properties.Enabled = false;
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
  EXPECT_FLOAT_EQ(panel->Group->Width, 350.0f);
  EXPECT_FLOAT_EQ(panel->Group->Height, 240.0f);
}
} // namespace z8::ui
