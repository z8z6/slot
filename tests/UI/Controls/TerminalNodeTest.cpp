#include "UI/Layout/TerminalNode.h"

#include "UI/Behavior/DockBehavior.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/PanelGroupNode.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(TerminalNodeTest, KeepsBoundedMessagesInStableTextNode) {
  TerminalNode terminal;
  auto *output = terminal.OutputNode;
  terminal.MaxLines = 2;

  terminal.AppendMessage("First");
  terminal.AppendMessage("Second");
  terminal.AppendMessage("Third");

  EXPECT_EQ(terminal.MessageCount(), 2U);
  EXPECT_EQ(terminal.OutputNode, output);
  EXPECT_EQ(terminal.OutputText(), "Second\nThird");
}

TEST(TerminalNodeTest, ReceivesPanelDragMessagesFromLayout) {
  Layout layout;
  auto panel = std::make_unique<PanelNode>();
  auto terminal = std::make_unique<TerminalNode>();
  auto *panelObserver = panel.get();
  auto *terminalObserver = terminal.get();
  terminal->GetBehavior<DockBehavior>()->Properties.Placement =
      DockPlacement::Bottom;
  terminal->GetBehavior<DockBehavior>()->Properties.Extent = 160.0f;
  layout.Root->AddChild(std::move(panel));
  layout.Root->AddChild(std::move(terminal));
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
  pointer.X = 790;
  pointer.Y = 100;
  pointer.DeltaX = pointer.X - startX;
  pointer.DeltaY = pointer.Y - startY;
  ASSERT_NE(layout.OnMouseDrag(pointer), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(pointer), EventReply::Ignored);

  EXPECT_EQ(terminalObserver->MessageCount(), 3U);
  EXPECT_NE(terminalObserver->OutputText().find("[Drag] Started: Panel"),
            std::string::npos);
  EXPECT_NE(terminalObserver->OutputText().find("[Drag] Moved: Panel"),
            std::string::npos);
  EXPECT_NE(terminalObserver->OutputText().find("[Drag] Completed: Panel"),
            std::string::npos);
}

TEST(TerminalNodeTest, ScrollsToBottomAfterAppendingMessages) {
  Layout layout;
  auto terminal = std::make_unique<TerminalNode>();
  auto *observer = terminal.get();
  observer->GetBehavior<DockBehavior>()->Properties.Enabled = false;
  terminal->Style.Width = 400.0f;
  terminal->Style.Height = 180.0f;
  terminal->Style.FlexGrow = 0.0f;
  terminal->Style.FlexShrink = 0.0f;
  layout.Root->AddChild(std::move(terminal));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  for (int i = 0; i < 20; ++i)
    observer->AppendMessage("Message " + std::to_string(i));
  layout.Calculate(800.0f, 600.0f);

  const auto *scroll = observer->ScrollAreaNode->GetScrollBehavior();
  ASSERT_GT(scroll->GetMaximumOffsetY(), 0.0f);
  EXPECT_FLOAT_EQ(scroll->GetOffsetY(), scroll->GetMaximumOffsetY());
  // 自动滚底必须在同一次 Calculate 内更新文字绝对坐标，渲染器不会为滚动
  // 偏移额外执行布局；末行底边应当已经进入 viewport，而不是滞后一帧。
  const auto *viewport = observer->ScrollAreaNode->ViewportNode;
  EXPECT_LE(observer->OutputNode->Top + observer->OutputNode->Height,
            viewport->Top + viewport->Height + 0.01f);
  EXPECT_GT(observer->OutputNode->Top + observer->OutputNode->Height,
            viewport->Top);
}

TEST(TerminalNodeTest, ReceivesTopPanelResizeMessages) {
  Layout layout;
  auto top = std::make_unique<PanelNode>();
  auto fill = std::make_unique<PanelNode>();
  auto terminal = std::make_unique<TerminalNode>();
  auto *topObserver = top.get();
  auto *terminalObserver = terminal.get();
  top->Key = "toolbar";
  top->GetBehavior<DockBehavior>()->Properties.Placement = DockPlacement::Top;
  top->GetBehavior<DockBehavior>()->Properties.Extent = 100.0f;
  top->SetProperty("MinHeight", "30");
  fill->GetBehavior<DockBehavior>()->Properties.Placement = DockPlacement::Fill;
  terminal->GetBehavior<DockBehavior>()->Properties.Placement =
      DockPlacement::Bottom;
  terminal->GetBehavior<DockBehavior>()->Properties.Extent = 160.0f;
  layout.Root->AddChild(std::move(top));
  layout.Root->AddChild(std::move(fill));
  layout.Root->AddChild(std::move(terminal));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  MouseMovArgs pointer;
  pointer.State = MK_LBUTTON;
  pointer.Button = MouseButton::Left;
  pointer.X = 400;
  pointer.Y = 100;
  ASSERT_NE(layout.OnMouseDown(pointer), EventReply::Ignored);
  pointer.Y = 140;
  pointer.DeltaY = 40;
  ASSERT_NE(layout.OnMouseDrag(pointer), EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(pointer), EventReply::Ignored);

  layout.Calculate(800.0f, 600.0f);
  ASSERT_NE(topObserver->Group, nullptr);
  EXPECT_FLOAT_EQ(topObserver->Group->Height, 140.0f);
  const auto *topLeaf = layout.Dock.Tree.FindPanelLeaf(topObserver);
  ASSERT_NE(topLeaf, nullptr);
  ASSERT_NE(topLeaf->Parent, nullptr);
  EXPECT_FLOAT_EQ(topLeaf->Parent->SplitRatio,
                  140.0f / topLeaf->Parent->Rect.Height);
  EXPECT_TRUE(layout.Dock.Tree.Validate());
}

} // namespace z8::ui
