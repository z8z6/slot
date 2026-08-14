#include "UI/Layout/Layout.h"
#include "UI/Layout/MenuNode.h"
#include "UI/Layout/SceneNode.h"
#include "UI/Layout/ToolBarNode.h"
#include "UI/Style/Theme.h"

#include <gtest/gtest.h>

namespace z8::ui {
namespace {

MouseMovArgs LeftPointer(const BaseNode &node) {
  MouseMovArgs args;
  args.State = MK_LBUTTON;
  args.Button = MouseButton::Left;
  args.X = static_cast<int>(node.Left + node.Width * 0.5f);
  args.Y = static_cast<int>(node.Top + node.Height * 0.5f);
  return args;
}

struct MenuFixture {
  ToolBarNode *ToolBar = nullptr;
  MenuNode *File = nullptr;
  MenuNode *Open = nullptr;
  MenuNode *Recent = nullptr;
  MenuItemNode *Leaf = nullptr;
  SceneNode *Scene = nullptr;
};

MenuFixture BuildMenu(Layout &layout) {
  MenuFixture result;
  auto toolbar = std::make_unique<ToolBarNode>();
  result.ToolBar = toolbar.get();
  toolbar->Key = "toolbar";

  auto file = std::make_unique<MenuNode>();
  result.File = file.get();
  file->SetText("File");
  auto open = std::make_unique<MenuNode>();
  result.Open = open.get();
  open->SetText("Open");
  auto recent = std::make_unique<MenuNode>();
  result.Recent = recent.get();
  recent->SetText("Recent");
  auto leaf = std::make_unique<MenuItemNode>();
  result.Leaf = leaf.get();
  leaf->SetText("Demo.slot");
  recent->ContentHost()->AddChild(std::move(leaf));
  open->ContentHost()->AddChild(std::move(recent));
  file->ContentHost()->AddChild(std::move(open));
  toolbar->AddChild(std::move(file));
  layout.Root->AddChild(std::move(toolbar));

  // Scene 故意后声明；Layout 仍须把 ToolBar Popup 放到最终画家顺序顶部。
  auto scene = std::make_unique<SceneNode>();
  result.Scene = scene.get();
  layout.Root->AddChild(std::move(scene));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);
  layout.Calculate(800.0f, 600.0f);
  return result;
}

} // namespace

TEST(MenuNodeTest, ToolBarDocksDirectlyAboveWorkspaceWithoutPanelGroup) {
  Layout layout;
  const auto fixture = BuildMenu(layout);

  EXPECT_TRUE(layout.Dock.IsDocked(*fixture.ToolBar));
  EXPECT_FLOAT_EQ(fixture.ToolBar->Height, Theme::Default().ToolBar.Height);
  EXPECT_FLOAT_EQ(fixture.Scene->Top, Theme::Default().ToolBar.Height);
  EXPECT_TRUE(fixture.File->IsTopLevel());
  EXPECT_FALSE(fixture.Open->IsTopLevel());
  EXPECT_EQ(fixture.Recent->Parent, fixture.Open->PopupNode);

  ASSERT_NE(layout.Dock.Tree.Root, nullptr);
  ASSERT_EQ(layout.Dock.Tree.Root->Type, DockNodeType::Split);
  EXPECT_FALSE(layout.Dock.Tree.Root->SplitterResizable);
  EXPECT_EQ(layout.Dock.Tree.FindSplitterAt(400.0f, fixture.ToolBar->Height),
            nullptr);
  EXPECT_FALSE(layout.Dock.Tree.ResizeSplitter(layout.Dock.Tree.Root->ID,
                                               400.0f, 80.0f));

  layout.Calculate(800.0f, 900.0f);
  EXPECT_FLOAT_EQ(fixture.ToolBar->Height, Theme::Default().ToolBar.Height);
  EXPECT_FLOAT_EQ(fixture.Scene->Top, Theme::Default().ToolBar.Height);
}

TEST(MenuNodeTest, OpensThreeLevelsAndLeafActivationClosesHierarchy) {
  Layout layout;
  const auto fixture = BuildMenu(layout);
  auto pointer = LeftPointer(*fixture.File);

  EXPECT_EQ(layout.OnMouseDown(pointer), EventReply::Handled);
  layout.OnMouseUp(pointer);
  layout.Calculate(800.0f, 600.0f);
  ASSERT_TRUE(fixture.File->Open);
  ASSERT_TRUE(fixture.File->PopupNode->EffectiveVisible);

  pointer = LeftPointer(*fixture.Open);
  // 第一层 Popup 跨越 ToolBar splitter；边界附近仍必须命中 Menu 而非 resize。
  pointer.Y = static_cast<int>(fixture.Open->Top + 2.0f);
  EXPECT_EQ(layout.OnMouseDown(pointer), EventReply::Handled);
  layout.OnMouseUp(pointer);
  ASSERT_TRUE(fixture.Open->Open);
  pointer = LeftPointer(*fixture.Open);
  EXPECT_EQ(layout.OnMouseMove(pointer), EventReply::Handled);
  layout.Calculate(800.0f, 600.0f);
  ASSERT_TRUE(fixture.Open->Open);

  pointer = LeftPointer(*fixture.Recent);
  EXPECT_EQ(layout.OnMouseMove(pointer), EventReply::Handled);
  layout.Calculate(800.0f, 600.0f);
  ASSERT_TRUE(fixture.Recent->Open);
  ASSERT_TRUE(fixture.Leaf->EffectiveVisible);

  pointer = LeftPointer(*fixture.Leaf);
  EXPECT_EQ(layout.OnMouseDown(pointer), EventReply::Handled);
  EXPECT_EQ(layout.CapturedHandler, fixture.Leaf);
  EXPECT_EQ(layout.OnMouseUp(pointer), EventReply::Handled);
  EXPECT_TRUE(fixture.Leaf->ConsumeClicked());
  EXPECT_FALSE(fixture.File->Open);
  EXPECT_FALSE(fixture.Open->Open);
  EXPECT_FALSE(fixture.Recent->Open);
}

TEST(MenuNodeTest, ClickingOutsideClosesOpenTopLevelMenu) {
  Layout layout;
  const auto fixture = BuildMenu(layout);
  fixture.File->SetOpen(true);
  layout.Calculate(800.0f, 600.0f);

  MouseMovArgs outside;
  outside.Button = MouseButton::Left;
  outside.X = 700;
  outside.Y = 300;
  layout.OnMouseDown(outside);
  EXPECT_FALSE(fixture.File->Open);

  fixture.File->SetOpen(true);
  layout.OnPointerCaptureLost();
  EXPECT_FALSE(fixture.File->Open);
}

TEST(MenuNodeTest, PopupOccludesEarlierPanelTextButKeepsMenuText) {
  Layout layout;
  const auto fixture = BuildMenu(layout);
  fixture.File->SetOpen(true);
  layout.Calculate(800.0f, 600.0f);

  ASSERT_NE(fixture.Scene->TitleNode, nullptr);
  ASSERT_NE(fixture.Open->LabelNode, nullptr);
  EXPECT_TRUE(fixture.Scene->TitleNode->HasTextOcclusion);
  EXPECT_FALSE(fixture.Open->LabelNode->HasTextOcclusion);

  const DirectX::XMFLOAT4 popup{
      fixture.File->PopupNode->Left, fixture.File->PopupNode->Top,
      fixture.File->PopupNode->Left + fixture.File->PopupNode->Width,
      fixture.File->PopupNode->Top + fixture.File->PopupNode->Height};
  for (const auto &clip : fixture.Scene->TitleNode->VisibleTextClips) {
    const bool intersects = clip.x < popup.z && clip.z > popup.x &&
                            clip.y < popup.w && clip.w > popup.y;
    EXPECT_FALSE(intersects);
  }
}

} // namespace z8::ui
