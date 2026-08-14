#include "UI/Layout/SceneNode.h"

#include "UI/Behavior/DockBehavior.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Layout/PanelNode.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(SceneNodeTest, DefinesFillViewportWithInteractiveTitle) {
  SceneNode scene;

  EXPECT_STREQ(scene.TypeName(), "Scene");
  EXPECT_TRUE(scene.HitTestVisible);
  ASSERT_NE(scene.TitleBarNode, nullptr);
  ASSERT_NE(scene.TitleNode, nullptr);
  ASSERT_NE(scene.ViewportNode, nullptr);
  EXPECT_EQ(scene.TitleNode->Text, "Viewport");
  EXPECT_FALSE(scene.TitleBarNode->RoutesToScene());
  EXPECT_TRUE(scene.ViewportNode->RoutesToScene());
  EXPECT_NE(scene.GetBehavior<DragBehavior>(), nullptr);
  EXPECT_NE(scene.GetBehavior<ResizeBehavior>(), nullptr);
  const auto *dock = scene.GetBehavior<DockBehavior>();
  ASSERT_NE(dock, nullptr);
  EXPECT_EQ(dock->Properties.Placement, DockPlacement::Fill);
  EXPECT_FLOAT_EQ(scene.Style.Margin, 0.0f);
}

TEST(SceneNodeTest, DragsFromTitleAndResizesFromBorder) {
  Layout layout;
  auto scene = std::make_unique<SceneNode>();
  auto *observer = scene.get();
  auto *dock = observer->GetBehavior<DockBehavior>();
  dock->Properties.Enabled = false;
  scene->Style.Width = 400.0f;
  scene->Style.Height = 300.0f;
  scene->Style.FlexGrow = 0.0f;
  scene->Style.FlexShrink = 0.0f;
  layout.Root->AddChild(std::move(scene));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  MouseMovArgs pointer;
  pointer.Button = MouseButton::Left;
  pointer.State = MK_LBUTTON;
  pointer.X = 100;
  pointer.Y = 15;
  ASSERT_NE(layout.OnMouseDown(pointer), EventReply::Ignored);
  pointer.X = 130;
  pointer.Y = 35;
  pointer.DeltaX = 30;
  pointer.DeltaY = 20;
  ASSERT_NE(layout.OnMouseDrag(pointer), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(pointer), EventReply::Ignored);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(observer->Left, 30.0f);
  EXPECT_FLOAT_EQ(observer->Top, 20.0f);
  const float draggedWidth = observer->Width;
  const float draggedHeight = observer->Height;

  pointer.X = static_cast<int>(observer->Viewport().Left + 20.0f);
  pointer.Y = static_cast<int>(observer->Viewport().Top + 20.0f);
  pointer.DeltaX = 0;
  pointer.DeltaY = 0;
  EXPECT_EQ(layout.OnMouseDown(pointer), EventReply::Ignored);

  pointer.X = static_cast<int>((std::min)(
      observer->Left + observer->Width - 1.0f, layout.Root->Width - 1.0f));
  pointer.Y = static_cast<int>((std::min)(
      observer->Top + observer->Height - 1.0f, layout.Root->Height - 1.0f));
  pointer.DeltaX = 0;
  pointer.DeltaY = 0;
  ASSERT_NE(layout.OnMouseDown(pointer), EventReply::Ignored);
  pointer.X = 479;
  pointer.Y = 359;
  pointer.DeltaX = 50;
  pointer.DeltaY = 40;
  ASSERT_NE(layout.OnMouseDrag(pointer), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(pointer), EventReply::Ignored);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(observer->Width, draggedWidth + 50.0f);
  EXPECT_FLOAT_EQ(observer->Height, draggedHeight + 40.0f);
}

TEST(SceneNodeTest, UsesPanelDockCommitWhenDraggedToFloatingCenter) {
  Layout layout;
  auto scene = std::make_unique<SceneNode>();
  auto *observer = scene.get();
  observer->Key = "scene";
  layout.Root->AddChild(std::move(scene));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  MouseMovArgs pointer;
  pointer.Button = MouseButton::Left;
  pointer.State = MK_LBUTTON;
  pointer.X = 100;
  pointer.Y = 16;
  ASSERT_NE(layout.OnMouseDown(pointer), EventReply::Ignored);
  pointer.X = 400;
  pointer.Y = 300;
  pointer.DeltaX = 300;
  pointer.DeltaY = 284;
  ASSERT_NE(layout.OnMouseDrag(pointer), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(pointer), EventReply::Ignored);

  const auto *state = layout.Dock.GetState(*observer);
  ASSERT_NE(state, nullptr);
  EXPECT_EQ(state->Placement, PanelPlacement::Floating);
  EXPECT_EQ(layout.Dock.Tree.FindPanelLeaf(observer), nullptr);
  const auto floating = layout.Dock.GetFloatingPanels();
  ASSERT_EQ(floating.size(), 1U);
  EXPECT_EQ(floating.front(), observer);
}

TEST(SceneNodeTest, UsesPanelDockCommitWhenDraggedToSiblingEdge) {
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
  pointer.Button = MouseButton::Left;
  pointer.State = MK_LBUTTON;
  pointer.X = static_cast<int>(sceneObserver->Left + 100.0f);
  pointer.Y = static_cast<int>(sceneObserver->Top + 16.0f);
  ASSERT_NE(layout.OnMouseDown(pointer), EventReply::Ignored);
  const int startX = pointer.X;
  const int startY = pointer.Y;
  pointer.X = static_cast<int>(panelObserver->Group->Left +
                               panelObserver->Group->Width * 0.8f);
  pointer.Y = static_cast<int>(panelObserver->Group->Top +
                               panelObserver->Group->Height * 0.5f);
  pointer.DeltaX = pointer.X - startX;
  pointer.DeltaY = pointer.Y - startY;
  ASSERT_NE(layout.OnMouseDrag(pointer), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(pointer), EventReply::Ignored);

  const auto *sceneLeaf = layout.Dock.Tree.FindPanelLeaf(sceneObserver);
  ASSERT_NE(sceneLeaf, nullptr);
  ASSERT_NE(sceneLeaf->Parent, nullptr);
  EXPECT_EQ(sceneLeaf->Parent->Axis, SplitAxis::Vertical);
  EXPECT_EQ(layout.Dock.GetState(*sceneObserver)->Placement,
            PanelPlacement::Docked);
}

} // namespace z8::ui
