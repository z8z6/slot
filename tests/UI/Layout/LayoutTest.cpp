#include "UI/Layout/Layout.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Layout/PanelNode.h"
#include "UI/Layout/RectNode.h"
#include "UI/Layout/SceneNode.h"
#include "UI/Style/Theme.h"

#include <gtest/gtest.h>
#include <ranges>

namespace z8::ui {
TEST(LayoutTest, BuildsNodeAndRenderableIndexes) {
  Layout layout;
  auto child = std::make_unique<RectNode>();
  child->Key = "content";
  layout.Root->AddChild(std::move(child));
  layout.RebuildIndex();

  EXPECT_EQ(layout.Nodes.size(), 2U);
  EXPECT_EQ(layout.Visuals.size(), 1U);
  const auto objects = layout.GetUO();
  ASSERT_EQ(objects.size(), 1U);
  EXPECT_NE(objects.front(), nullptr);
  EXPECT_NE(layout.Find("content"), nullptr);
  EXPECT_TRUE(layout.ConsumeDirty());
  EXPECT_FALSE(layout.ConsumeDirty());
}

TEST(LayoutTest, SeparatesFloatingSubtreeFromMainWindowDrawLists) {
  Layout layout;
  auto first = std::make_unique<PanelNode>();
  auto second = std::make_unique<PanelNode>();
  auto *firstPanel = first.get();
  layout.Root->AddChild(std::move(first));
  layout.Root->AddChild(std::move(second));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  auto *group = firstPanel->Group;
  ASSERT_NE(group, nullptr);

  layout.Dock.BeginDrag(*group, group->DragHandleNode->Left + 8.0f,
                        group->DragHandleNode->Top + 8.0f);
  layout.Dock.UpdateDrag(900.0f, 300.0f);
  ASSERT_TRUE(layout.Dock.CommitDrag(900.0f, 300.0f));
  layout.Calculate(800.0f, 600.0f);

  const auto mainObjects = layout.GetMainUO();
  const auto floatingObjects = layout.GetSubtreeUO(*group);
  ASSERT_FALSE(floatingObjects.empty());
  for (auto *object : floatingObjects)
    EXPECT_EQ(std::ranges::find(mainObjects, object), mainObjects.end());
  EXPECT_FALSE(layout.GetSubtreeTexts(*group).empty());
}

TEST(LayoutTest, CalculatesWithoutApplicationOrWindow) {
  Layout layout;
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(layout.Root->Computed.Width, 800.0f);
  EXPECT_FLOAT_EQ(layout.Root->Computed.Height, 600.0f);
}

TEST(LayoutTest, TogglesRedPanelBorderDebugModeWithNumberThree) {
  Layout layout;
  auto panel = std::make_unique<PanelNode>();
  layout.Root->AddChild(std::move(panel));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  EXPECT_EQ(layout.GetPanelDebugBorderCount(), 0U);

  EXPECT_EQ(layout.OnKeyDown(KeyArgs('3')), EventReply::Handled);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_TRUE(layout.DebugPanelBorders);
  // 自动 PanelGroup、页签、图标、标题文字、滚动 viewport/content、滚动条
  // 与滑块都应拥有独立诊断框，而不是只显示最外层 Panel 边界。
  EXPECT_GT(layout.GetPanelDebugBorderCount(), 8U);

  EXPECT_EQ(layout.OnKeyDown(KeyArgs('3')), EventReply::Handled);
  EXPECT_FALSE(layout.DebugPanelBorders);
  EXPECT_EQ(layout.GetPanelDebugBorderCount(), 0U);
}

TEST(LayoutTest, RoutesSceneViewportInputPastUIOverlay) {
  Layout layout;
  auto scene = std::make_unique<SceneNode>();
  scene->Key = "viewport";
  layout.Root->AddChild(std::move(scene));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  MouseMovArgs pointer;
  pointer.X = 400;
  pointer.Y = 300;
  MouseWheelArgs wheel;
  wheel.X = 400;
  wheel.Y = 300;
  wheel.Delta = 120;
  EXPECT_EQ(layout.OnMouseDown(pointer), EventReply::Ignored);
  EXPECT_EQ(layout.OnMouseMove(pointer), EventReply::Ignored);
  EXPECT_EQ(layout.OnMouseWheel(wheel), EventReply::Ignored);
}

TEST(LayoutTest, ReservesEditorPanelsAroundSceneViewport) {
  Layout layout;
  auto addPanel = [&layout](const char *key, DockPlacement placement,
                            float extent) -> PanelNode * {
    auto panel = std::make_unique<PanelNode>();
    auto *observer = panel.get();
    panel->Key = key;
    auto *dock = panel->GetBehavior<DockBehavior>();
    dock->Properties.Placement = placement;
    dock->Properties.Extent = extent;
    layout.Root->AddChild(std::move(panel));
    return observer;
  };
  auto *toolbar = addPanel("toolbar", DockPlacement::Top, 48.0f);
  toolbar->Style.MinHeight = 34.0f;
  addPanel("drawer", DockPlacement::Bottom, 180.0f);
  addPanel("outliner", DockPlacement::Left, 260.0f);
  addPanel("details", DockPlacement::Right, 300.0f);
  auto scene = std::make_unique<SceneNode>();
  auto *sceneObserver = scene.get();
  layout.Root->AddChild(std::move(scene));
  layout.RebuildIndex();

  layout.Calculate(1200.0f, 800.0f);

  EXPECT_FLOAT_EQ(sceneObserver->Left, 260.0f);
  EXPECT_FLOAT_EQ(sceneObserver->Top, 48.0f);
  EXPECT_FLOAT_EQ(sceneObserver->Width, 640.0f);
  EXPECT_FLOAT_EQ(sceneObserver->Height, 572.0f);
  ASSERT_NE(toolbar->Group, nullptr);
  EXPECT_FLOAT_EQ(toolbar->Group->Height, 48.0f);
  EXPECT_FLOAT_EQ(toolbar->Height, toolbar->Group->PagesNode->Height);
  EXPECT_FLOAT_EQ(toolbar->ScrollAreaNode->Height,
                  toolbar->Group->PagesNode->Height);
  EXPECT_GT(toolbar->ScrollAreaNode->Height, 0.0f);
  const float titleHeight = Theme::Default().Panel.TitleHeight;
  EXPECT_FLOAT_EQ(sceneObserver->Viewport().Top, 48.0f + titleHeight);
  EXPECT_FLOAT_EQ(sceneObserver->Viewport().Height, 572.0f - titleHeight);
}

TEST(LayoutTest, DocksPanelWhenDroppedOverSceneViewport) {
  Layout layout;
  auto panel = std::make_unique<PanelNode>();
  auto scene = std::make_unique<SceneNode>();
  auto *panelObserver = panel.get();
  auto *sceneObserver = scene.get();
  layout.Root->AddChild(std::move(panel));
  layout.Root->AddChild(std::move(scene));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  MouseMovArgs pointer;
  pointer.State = MK_LBUTTON;
  pointer.Button = MouseButton::Left;
  pointer.X =
      static_cast<int>(panelObserver->Group->Tabs.front()->Left + 12.0f);
  pointer.Y =
      static_cast<int>(panelObserver->Group->Tabs.front()->Top + 12.0f);
  const int startX = pointer.X;
  const int startY = pointer.Y;
  ASSERT_NE(layout.OnMouseDown(pointer), EventReply::Ignored);
  pointer.X =
      static_cast<int>(sceneObserver->Left + sceneObserver->Width * 0.8f);
  pointer.Y =
      static_cast<int>(sceneObserver->Top + sceneObserver->Height * 0.5f);
  pointer.DeltaX = pointer.X - startX;
  pointer.DeltaY = pointer.Y - startY;
  ASSERT_NE(layout.OnMouseDrag(pointer), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(pointer), EventReply::Ignored);

  const auto *panelLeaf = layout.Dock.Tree.FindPanelLeaf(panelObserver);
  ASSERT_NE(panelLeaf, nullptr);
  EXPECT_EQ(panelLeaf->Parent->Axis, SplitAxis::Vertical);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_GE(panelObserver->Left, sceneObserver->Left + sceneObserver->Width);
}
} // namespace z8::ui
