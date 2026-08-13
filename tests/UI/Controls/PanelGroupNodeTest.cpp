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
  ASSERT_NE(layout.OnMouseDown(LeftClick(300, 15)), EventReply::Ignored);
  EXPECT_EQ(observer->ActivePanel, 1U);
  EXPECT_FALSE(observer->Panels[0]->Visible);
  EXPECT_TRUE(observer->Panels[1]->Visible);
}

TEST(PanelGroupNodeTest, DraggingTabToAnotherTitleBarMergesGroups) {
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
  auto *targetGroup = secondPanel->Group;

  ASSERT_NE(layout.OnMouseDown(LeftClick(100, 15)), EventReply::Ignored);
  auto drag = LeftClick(600, 15);
  drag.DeltaX = 500;
  ASSERT_NE(layout.OnMouseDrag(drag), EventReply::Ignored);
  EXPECT_TRUE(layout.Dock.Drag.TargetGroupTitle);
  ASSERT_NE(layout.OnMouseUp(drag), EventReply::Ignored);

  ASSERT_EQ(targetGroup->Panels.size(), 2U);
  EXPECT_EQ(firstPanel->Group, targetGroup);
  EXPECT_EQ(secondPanel->Group, targetGroup);
  EXPECT_EQ(targetGroup->Panels[targetGroup->ActivePanel], firstPanel);
  EXPECT_TRUE(layout.Dock.Tree.Validate());
}

} // namespace z8::ui
