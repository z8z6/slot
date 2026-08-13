#include "UI/Layout/Layout.h"
#include "UI/Layout/PanelGroupNode.h"
#include "Object/UIObject/UIObject.h"

#include <gtest/gtest.h>

namespace z8::ui {
namespace {

std::unique_ptr<PanelNode> MakePanel(const std::string &title) {
  auto panel = std::make_unique<PanelNode>();
  panel->SetProperty("Title", title);
  return panel;
}

MouseMovArgs LeftClick(int x, int y) {
  MouseMovArgs args;
  args.X = x;
  args.Y = y;
  args.Button = MouseButton::Left;
  args.State = MK_LBUTTON;
  return args;
}

} // namespace

TEST(PanelGroupNodeTest, OwnsPanelsAndOnlyShowsActivePage) {
  PanelGroupNode group;
  auto *first = group.AddPanel(MakePanel("Scene"));
  auto *second = group.AddPanel(MakePanel("Game"));

  ASSERT_NE(first, nullptr);
  ASSERT_NE(second, nullptr);
  ASSERT_EQ(group.Panels.size(), 2U);
  ASSERT_EQ(group.Tabs.size(), 2U);
  EXPECT_TRUE(first->Visible);
  EXPECT_FALSE(second->Visible);
  EXPECT_FALSE(first->TitleBarNode->Visible);
  EXPECT_EQ(first->Parent, group.PagesNode);

  ASSERT_TRUE(group.ActivatePanel(1));
  EXPECT_FALSE(first->Visible);
  EXPECT_TRUE(second->Visible);
  EXPECT_NE(group.Tabs[0]->UO->GetColor().x,
            group.Tabs[1]->UO->GetColor().x);
}

TEST(PanelGroupNodeTest, SizesTabsFromTitleTextAndMarksActiveTab) {
  Layout layout;
  auto group = std::make_unique<PanelGroupNode>();
  auto *observer = group.get();
  observer->GetBehavior<DockBehavior>()->Properties.Enabled = false;
  observer->Style.Margin = 0.0f;
  observer->Style.Width = 600.0f;
  observer->Style.Height = 300.0f;
  observer->AddPanel(MakePanel("Scene"));
  observer->AddPanel(MakePanel("Inspector Properties"));
  layout.Root->AddChild(std::move(group));
  layout.RebuildIndex();
  layout.Calculate(600.0f, 300.0f);

  ASSERT_EQ(observer->Tabs.size(), 2U);
  EXPECT_LT(observer->Tabs[0]->Width, observer->Tabs[1]->Width);
  EXPECT_LT(observer->Tabs[1]->Width, observer->Width);
  EXPECT_TRUE(observer->Tabs[0]->SelectionNode->Visible);
  EXPECT_FALSE(observer->Tabs[1]->SelectionNode->Visible);
  ASSERT_TRUE(observer->ActivatePanel(1));
  EXPECT_FALSE(observer->Tabs[0]->SelectionNode->Visible);
  EXPECT_TRUE(observer->Tabs[1]->SelectionNode->Visible);
}

TEST(PanelGroupNodeTest, UpdatesTabWidthAndIconWhenPanelPropertiesChange) {
  PanelGroupNode group;
  auto *panel = group.AddPanel(MakePanel("A"));
  ASSERT_NE(panel, nullptr);
  ASSERT_EQ(group.Tabs.size(), 1U);
  const float shortWidth = group.Tabs.front()->Style.Width.value();

  ASSERT_TRUE(panel->SetProperty("Title", "Inspector Properties"));
  EXPECT_GT(group.Tabs.front()->Style.Width.value(), shortWidth);
  EXPECT_EQ(group.Tabs.front()->LabelNode->Text, "Inspector Properties");

  ASSERT_TRUE(panel->SetProperty(
      "Icon", "asset://texture/icons/lucide/settings-2.svg"));
  EXPECT_EQ(group.Tabs.front()->IconNode->Kind, ImageKind::Settings);
}

TEST(PanelGroupNodeTest, SwitchesPageByClickingTabTitle) {
  Layout layout;
  auto group = std::make_unique<PanelGroupNode>();
  auto *observer = group.get();
  observer->GetBehavior<DockBehavior>()->Properties.Enabled = false;
  observer->Style.Margin = 0.0f;
  observer->Style.Width = 400.0f;
  observer->Style.Height = 300.0f;
  observer->AddPanel(MakePanel("Scene"));
  observer->AddPanel(MakePanel("Game"));
  layout.Root->AddChild(std::move(group));
  layout.RebuildIndex();
  layout.Calculate(400.0f, 300.0f);

  ASSERT_EQ(observer->ActivePanel, 0U);
  ASSERT_NE(layout.OnMouseDown(LeftClick(100, 15)), EventReply::Ignored);
  EXPECT_EQ(observer->ActivePanel, 1U);
  EXPECT_FALSE(observer->Panels[0]->Visible);
  EXPECT_TRUE(observer->Panels[1]->Visible);
  EXPECT_EQ(layout.Dock.Drag.State, PanelDragState::Idle);
}

TEST(PanelGroupNodeTest, MouseDownChoosesPanelOrGroupPayloadFromHitRegion) {
  Layout layout;
  auto group = std::make_unique<PanelGroupNode>();
  auto *observer = group.get();
  observer->Style.Margin = 0.0f;
  observer->AddPanel(MakePanel("Scene"));
  observer->AddPanel(MakePanel("Game"));
  layout.Root->AddChild(std::move(group));
  layout.RebuildIndex();
  layout.Calculate(600.0f, 300.0f);

  ASSERT_NE(layout.OnMouseDown(LeftClick(20, 15)), EventReply::Ignored);
  EXPECT_EQ(layout.Dock.Drag.PayloadType, DragPayloadType::Panel);
  layout.OnPointerCaptureLost();

  ASSERT_NE(layout.OnMouseDown(LeftClick(300, 15)), EventReply::Ignored);
  EXPECT_EQ(layout.Dock.Drag.PayloadType, DragPayloadType::PanelGroup);
  EXPECT_EQ(layout.Dock.Drag.PayloadGroup, observer);
  layout.OnPointerCaptureLost();
}

TEST(PanelGroupNodeTest, PanelDragPreviewDoesNotMutateMembershipOrDockTree) {
  Layout layout;
  auto group = std::make_unique<PanelGroupNode>();
  auto *observer = group.get();
  auto *scene = group->AddPanel(MakePanel("Scene"));
  group->AddPanel(MakePanel("Game"));
  layout.Root->AddChild(std::move(group));
  layout.RebuildIndex();
  layout.Calculate(600.0f, 300.0f);
  const auto treeBefore = layout.Dock.Tree.Dump();
  const float tabLeftBefore = observer->Tabs.front()->Left;
  const float tabTopBefore = observer->Tabs.front()->Top;
  const float titleLeftBefore = observer->Tabs.front()->LabelNode->Left;
  const float titleTopBefore = observer->Tabs.front()->LabelNode->Top;

  ASSERT_NE(layout.OnMouseDown(LeftClick(20, 15)), EventReply::Ignored);
  auto drag = LeftClick(580, 150);
  drag.DeltaX = 560;
  drag.DeltaY = 135;
  ASSERT_NE(layout.OnMouseDrag(drag), EventReply::Ignored);

  EXPECT_EQ(layout.Dock.Drag.PayloadType, DragPayloadType::Panel);
  EXPECT_EQ(layout.Dock.Drag.State, PanelDragState::Dragging);
  EXPECT_EQ(observer->Panels.size(), 2U);
  EXPECT_EQ(scene->Group, observer);
  EXPECT_EQ(layout.Dock.Tree.Dump(), treeBefore);
  // 预览只能移动独立覆盖层；源 Tab 和标题文字不能被 DragBehavior 改为
  // 绝对定位后跟随鼠标，否则取消拖动也无法恢复原布局。
  EXPECT_FLOAT_EQ(observer->Tabs.front()->Left, tabLeftBefore);
  EXPECT_FLOAT_EQ(observer->Tabs.front()->Top, tabTopBefore);
  EXPECT_FLOAT_EQ(observer->Tabs.front()->LabelNode->Left, titleLeftBefore);
  EXPECT_FLOAT_EQ(observer->Tabs.front()->LabelNode->Top, titleTopBefore);
  layout.OnPointerCaptureLost();
}

TEST(PanelGroupNodeTest, PanelDropOnEmptyGroupHeaderJoinsTargetTabs) {
  Layout layout;
  auto source = std::make_unique<PanelGroupNode>();
  auto target = std::make_unique<PanelGroupNode>();
  auto *sourceGroup = source.get();
  auto *targetGroup = target.get();
  auto *scene = source->AddPanel(MakePanel("Scene"));
  source->AddPanel(MakePanel("Game"));
  target->AddPanel(MakePanel("Inspector"));
  layout.Root->AddChild(std::move(source));
  layout.Root->AddChild(std::move(target));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  const int startX = static_cast<int>(sourceGroup->Tabs.front()->Left + 12.0f);
  const int startY = static_cast<int>(sourceGroup->Tabs.front()->Top + 12.0f);
  const int targetX = static_cast<int>(targetGroup->DragHandleNode->Left +
                                       targetGroup->DragHandleNode->Width * 0.5f);
  const int targetY = static_cast<int>(targetGroup->HeaderNode->Top + 12.0f);
  ASSERT_NE(layout.OnMouseDown(LeftClick(startX, startY)),
            EventReply::Ignored);
  auto drag = LeftClick(targetX, targetY);
  drag.DeltaX = targetX - startX;
  drag.DeltaY = targetY - startY;
  ASSERT_NE(layout.OnMouseDrag(drag), EventReply::Ignored);

  EXPECT_EQ(layout.Dock.Drag.DockTarget,
            layout.Dock.Tree.FindPanelLeaf(targetGroup)->ID);
  EXPECT_EQ(layout.Dock.Drag.Side, DockSide::Center);
  ASSERT_NE(layout.OnMouseUp(drag), EventReply::Ignored);

  ASSERT_EQ(sourceGroup->Panels.size(), 1U);
  ASSERT_EQ(targetGroup->Panels.size(), 2U);
  EXPECT_EQ(scene->Group, targetGroup);
  EXPECT_EQ(targetGroup->Panels[targetGroup->ActivePanel], scene);
  EXPECT_TRUE(layout.Dock.Validate());
}

TEST(PanelGroupNodeTest, PanelCenterDropMovesTabToTargetGroup) {
  Layout layout;
  auto source = std::make_unique<PanelGroupNode>();
  auto target = std::make_unique<PanelGroupNode>();
  auto *sourceGroup = source.get();
  auto *targetGroup = target.get();
  auto *scene = source->AddPanel(MakePanel("Scene"));
  source->AddPanel(MakePanel("Game"));
  target->AddPanel(MakePanel("Inspector"));
  layout.Root->AddChild(std::move(source));
  layout.Root->AddChild(std::move(target));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  ASSERT_NE(layout.OnMouseDown(LeftClick(20, 15)), EventReply::Ignored);
  auto drag = LeftClick(600, 300);
  drag.DeltaX = 580;
  drag.DeltaY = 285;
  ASSERT_NE(layout.OnMouseDrag(drag), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(drag), EventReply::Ignored);

  ASSERT_EQ(sourceGroup->Panels.size(), 1U);
  ASSERT_EQ(targetGroup->Panels.size(), 2U);
  EXPECT_EQ(scene->Group, targetGroup);
  EXPECT_EQ(targetGroup->Panels[targetGroup->ActivePanel], scene);
  EXPECT_TRUE(layout.Dock.Validate());
}

TEST(PanelGroupNodeTest, PanelCenterDropIntoSourceGroupIsNoOp) {
  Layout layout;
  auto group = std::make_unique<PanelGroupNode>();
  auto *observer = group.get();
  auto *scene = group->AddPanel(MakePanel("Scene"));
  group->AddPanel(MakePanel("Game"));
  layout.Root->AddChild(std::move(group));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  ASSERT_NE(layout.OnMouseDown(LeftClick(20, 15)), EventReply::Ignored);
  auto drag = LeftClick(400, 300);
  drag.DeltaX = 380;
  drag.DeltaY = 285;
  ASSERT_NE(layout.OnMouseDrag(drag), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(drag), EventReply::Ignored);

  ASSERT_EQ(observer->Panels.size(), 2U);
  EXPECT_EQ(observer->Panels.front(), scene);
  EXPECT_EQ(scene->Group, observer);
  EXPECT_TRUE(layout.Dock.Validate());
}

TEST(PanelGroupNodeTest, PanelDropOnSiblingTabSwapsOrder) {
  Layout layout;
  auto group = std::make_unique<PanelGroupNode>();
  auto *observer = group.get();
  auto *scene = group->AddPanel(MakePanel("Scene"));
  auto *game = group->AddPanel(MakePanel("Game"));
  auto *inspector = group->AddPanel(MakePanel("Inspector"));
  ASSERT_TRUE(group->ActivatePanel(0));
  layout.Root->AddChild(std::move(group));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  const int startX = static_cast<int>(observer->Tabs[0]->Left + 12.0f);
  const int startY = static_cast<int>(observer->Tabs[0]->Top + 12.0f);
  const int targetX = static_cast<int>(observer->Tabs[2]->Left + 12.0f);
  const int targetY = static_cast<int>(observer->Tabs[2]->Top + 12.0f);
  ASSERT_NE(layout.OnMouseDown(LeftClick(startX, startY)),
            EventReply::Ignored);
  auto drag = LeftClick(targetX, targetY);
  drag.DeltaX = targetX - startX;
  drag.DeltaY = targetY - startY;
  ASSERT_NE(layout.OnMouseDrag(drag), EventReply::Ignored);
  EXPECT_EQ(layout.Dock.Drag.TargetTabIndex, 2);
  ASSERT_NE(layout.OnMouseUp(drag), EventReply::Ignored);

  ASSERT_EQ(observer->Panels.size(), 3U);
  EXPECT_EQ(observer->Panels[0], inspector);
  EXPECT_EQ(observer->Panels[1], game);
  EXPECT_EQ(observer->Panels[2], scene);
  EXPECT_EQ(observer->ActivePanel, 2U);
  EXPECT_EQ(observer->Tabs[2]->PanelIndex, 2U);
  EXPECT_TRUE(layout.Dock.Validate());
}

TEST(PanelGroupNodeTest, FloatingGroupCanSwapPanelTabs) {
  Layout layout;
  auto group = std::make_unique<PanelGroupNode>();
  auto *observer = group.get();
  auto *scene = group->AddPanel(MakePanel("Scene"));
  auto *game = group->AddPanel(MakePanel("Game"));
  layout.Root->AddChild(std::move(group));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  layout.Dock.BeginDrag(*observer, 300.0f, 15.0f);
  layout.Dock.UpdateDrag(900.0f, 300.0f);
  ASSERT_TRUE(layout.Dock.CommitDrag(900.0f, 300.0f));
  layout.Calculate(800.0f, 600.0f);

  const int startX = static_cast<int>(observer->Tabs[0]->Left + 12.0f);
  const int startY = static_cast<int>(observer->Tabs[0]->Top + 12.0f);
  const int targetX = static_cast<int>(observer->Tabs[1]->Left + 12.0f);
  const int targetY = static_cast<int>(observer->Tabs[1]->Top + 12.0f);
  ASSERT_NE(layout.OnMouseDown(LeftClick(startX, startY)),
            EventReply::Ignored);
  auto drag = LeftClick(targetX, targetY);
  drag.DeltaX = targetX - startX;
  drag.DeltaY = targetY - startY;
  ASSERT_NE(layout.OnMouseDrag(drag), EventReply::Ignored);
  ASSERT_EQ(layout.Dock.Drag.TargetGroup, observer);
  ASSERT_NE(layout.OnMouseUp(drag), EventReply::Ignored);

  EXPECT_EQ(observer->Panels[0], game);
  EXPECT_EQ(observer->Panels[1], scene);
  EXPECT_TRUE(layout.Dock.Validate());
}

TEST(PanelGroupNodeTest, PanelEdgeDropCreatesDockedSinglePanelGroup) {
  Layout layout;
  auto source = std::make_unique<PanelGroupNode>();
  auto target = std::make_unique<PanelGroupNode>();
  auto *sourceGroup = source.get();
  auto *scene = source->AddPanel(MakePanel("Scene"));
  source->AddPanel(MakePanel("Game"));
  target->AddPanel(MakePanel("Inspector"));
  layout.Root->AddChild(std::move(source));
  layout.Root->AddChild(std::move(target));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  ASSERT_NE(layout.OnMouseDown(LeftClick(20, 15)), EventReply::Ignored);
  auto drag = LeftClick(790, 300);
  drag.DeltaX = 770;
  drag.DeltaY = 285;
  ASSERT_NE(layout.OnMouseDrag(drag), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(drag), EventReply::Ignored);

  ASSERT_NE(scene->Group, nullptr);
  EXPECT_NE(scene->Group, sourceGroup);
  ASSERT_EQ(scene->Group->Panels.size(), 1U);
  ASSERT_NE(layout.Dock.GetState(*scene->Group), nullptr);
  EXPECT_EQ(layout.Dock.GetState(*scene->Group)->Placement,
            PanelPlacement::Docked);
  EXPECT_TRUE(layout.Dock.Validate());
}

TEST(PanelGroupNodeTest, PanelOutsideDropCreatesFloatingSinglePanelGroup) {
  Layout layout;
  auto source = std::make_unique<PanelGroupNode>();
  auto *sourceGroup = source.get();
  auto *scene = source->AddPanel(MakePanel("Scene"));
  source->AddPanel(MakePanel("Game"));
  layout.Root->AddChild(std::move(source));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  ASSERT_NE(layout.OnMouseDown(LeftClick(20, 15)), EventReply::Ignored);
  auto drag = LeftClick(900, 300);
  drag.DeltaX = 880;
  drag.DeltaY = 285;
  ASSERT_NE(layout.OnMouseDrag(drag), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(drag), EventReply::Ignored);

  ASSERT_NE(scene->Group, nullptr);
  EXPECT_NE(scene->Group, sourceGroup);
  ASSERT_EQ(scene->Group->Panels.size(), 1U);
  ASSERT_NE(layout.Dock.GetState(*scene->Group), nullptr);
  EXPECT_EQ(layout.Dock.GetState(*scene->Group)->Placement,
            PanelPlacement::Floating);
  EXPECT_TRUE(layout.Dock.Validate());
}

TEST(PanelGroupNodeTest, DraggingLastPanelRemovesEmptySourceAndCollapsesTree) {
  Layout layout;
  auto source = std::make_unique<PanelGroupNode>();
  auto target = std::make_unique<PanelGroupNode>();
  auto *sourceGroup = source.get();
  auto *scene = source->AddPanel(MakePanel("Scene"));
  target->AddPanel(MakePanel("Inspector"));
  layout.Root->AddChild(std::move(source));
  layout.Root->AddChild(std::move(target));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  ASSERT_NE(layout.OnMouseDown(LeftClick(20, 15)), EventReply::Ignored);
  auto drag = LeftClick(900, 300);
  drag.DeltaX = 880;
  drag.DeltaY = 285;
  ASSERT_NE(layout.OnMouseDrag(drag), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(drag), EventReply::Ignored);

  EXPECT_EQ(scene->Group == sourceGroup, false);
  EXPECT_EQ(layout.Root->Children.size(), 2U);
  ASSERT_NE(layout.Dock.Tree.Root, nullptr);
  EXPECT_EQ(layout.Dock.Tree.Root->Type, DockNodeType::Leaf);
  EXPECT_EQ(layout.Dock.GetState(*scene->Group)->Placement,
            PanelPlacement::Floating);
  EXPECT_TRUE(layout.Dock.Validate());
}

TEST(PanelGroupNodeTest, RemovingActivePanelSelectsAdjacentPage) {
  PanelGroupNode group;
  auto *first = group.AddPanel(MakePanel("Scene"));
  auto *second = group.AddPanel(MakePanel("Game"));
  auto *third = group.AddPanel(MakePanel("Inspector"));
  ASSERT_TRUE(group.ActivatePanel(1));

  auto removed = group.RemovePanel(1);

  ASSERT_NE(removed, nullptr);
  EXPECT_EQ(removed.get(), second);
  ASSERT_EQ(group.Panels.size(), 2U);
  EXPECT_EQ(group.Panels[group.ActivePanel], third);
  EXPECT_FALSE(first->Visible);
  EXPECT_TRUE(third->Visible);
  EXPECT_EQ(removed->Group, nullptr);
}

TEST(PanelGroupNodeTest, RemovingLastPanelRequestsGroupClose) {
  PanelGroupNode group;
  group.AddPanel(MakePanel("Scene"));

  auto removed = group.RemovePanel(0);

  ASSERT_NE(removed, nullptr);
  EXPECT_TRUE(group.Panels.empty());
  EXPECT_TRUE(group.CloseRequested);
}

TEST(PanelGroupNodeTest, CenterDropFloatsWholeGroupWithoutMergingTabs) {
  Layout layout;
  auto first = std::make_unique<PanelNode>();
  auto second = std::make_unique<PanelNode>();
  auto *firstPanel = first.get();
  auto *secondPanel = second.get();
  first->SetProperty("Title", "Scene");
  second->SetProperty("Title", "Game");
  layout.Root->AddChild(std::move(first));
  layout.Root->AddChild(std::move(second));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  ASSERT_NE(firstPanel->Group, nullptr);
  ASSERT_NE(secondPanel->Group, nullptr);
  auto *sourceGroup = firstPanel->Group;
  auto *targetGroup = secondPanel->Group;

  // 单页 Tab 占据标题栏左侧，空白标题区才是整组拖动入口。
  ASSERT_NE(layout.OnMouseDown(LeftClick(200, 15)), EventReply::Ignored);
  auto drag = LeftClick(600, 300);
  drag.DeltaX = 400;
  drag.DeltaY = 285;
  ASSERT_NE(layout.OnMouseDrag(drag), EventReply::Ignored);
  EXPECT_EQ(layout.Dock.Drag.Side, DockSide::Center);
  ASSERT_NE(layout.OnMouseUp(drag), EventReply::Ignored);

  ASSERT_EQ(sourceGroup->Panels.size(), 1U);
  ASSERT_EQ(targetGroup->Panels.size(), 1U);
  EXPECT_EQ(firstPanel->Group, sourceGroup);
  EXPECT_EQ(secondPanel->Group, targetGroup);
  ASSERT_NE(layout.Dock.GetState(*sourceGroup), nullptr);
  EXPECT_EQ(layout.Dock.GetState(*sourceGroup)->Placement,
            PanelPlacement::Floating);
  EXPECT_TRUE(layout.Dock.Tree.Validate());
}

TEST(PanelGroupNodeTest, CloseButtonRemovesDockedGroupAndCollapsesTree) {
  Layout layout;
  auto first = std::make_unique<PanelNode>();
  auto second = std::make_unique<PanelNode>();
  auto *secondPanel = second.get();
  layout.Root->AddChild(std::move(first));
  layout.Root->AddChild(std::move(second));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  ASSERT_NE(secondPanel->Group, nullptr);
  ASSERT_EQ(layout.Root->Children.size(), 2U);

  // 首个 Group 位于左半区，关闭按钮固定占据标题栏最右侧。
  ASSERT_NE(layout.OnMouseDown(LeftClick(380, 15)), EventReply::Ignored);

  ASSERT_EQ(layout.Root->Children.size(), 1U);
  ASSERT_NE(layout.Dock.Tree.Root, nullptr);
  EXPECT_EQ(layout.Dock.Tree.Root->Type, DockNodeType::Leaf);
  EXPECT_EQ(layout.Dock.Tree.FindPanelLeaf(secondPanel), layout.Dock.Tree.Root.get());
  EXPECT_EQ(layout.Dock.Drag.State, PanelDragState::Idle);
  EXPECT_TRUE(layout.Dock.Tree.Validate());
}

TEST(PanelGroupNodeTest, CloseButtonRemovesFloatingGroup) {
  Layout layout;
  auto panel = std::make_unique<PanelNode>();
  auto *panelObserver = panel.get();
  layout.Root->AddChild(std::move(panel));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  auto *group = panelObserver->Group;
  ASSERT_NE(group, nullptr);

  layout.Dock.BeginDrag(*group, 100.0f, 15.0f);
  layout.Dock.UpdateDrag(900.0f, 300.0f);
  ASSERT_TRUE(layout.Dock.CommitDrag(900.0f, 300.0f));
  ASSERT_EQ(layout.Dock.GetState(*group)->Placement, PanelPlacement::Floating);
  group->CloseRequested = true;
  layout.RebuildIndex();

  EXPECT_TRUE(layout.Root->Children.empty());
  EXPECT_EQ(layout.Dock.Tree.Root, nullptr);
  EXPECT_EQ(layout.Dock.Drag.State, PanelDragState::Idle);
  EXPECT_TRUE(layout.Dock.Validate());
}

} // namespace z8::ui
