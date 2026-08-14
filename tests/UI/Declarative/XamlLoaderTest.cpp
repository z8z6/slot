#include "UI/Declarative/XamlLoader.h"
#include "Object/UIObject/UIObject.h"
#include "UI/Layout/ButtonNode.h"
#include "UI/Layout/FileExplorerNode.h"
#include "UI/Layout/ImageNode.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/MenuNode.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Layout/PanelNode.h"
#include "UI/Layout/SceneNode.h"
#include "UI/Layout/SliderNode.h"
#include "UI/Layout/TerminalNode.h"
#include "UI/Layout/TextInputNode.h"
#include "UI/Layout/ToggleNode.h"
#include "UI/Layout/ToolBarNode.h"
#include "UI/Layout/TreeViewNode.h"

#include <gtest/gtest.h>

namespace z8::ui {
TEST(XamlLoaderTest, BuildsPanelControlTree) {
  constexpr auto source = R"(
    <?xml version="1.0"?>
    <UI Direction="Column">
      <Panel Id="tools" Title="Tools &amp; Scene" Width="300" Height="200"
             TitleHeight="40" Padding="4" DragRegion="Anywhere"
             Scrollable="true" HorizontalScrollEnabled="true"
             HorizontalScrollBar="Auto" VerticalScrollBar="Visible">
        <Rect Id="content" FlexGrow="1" />
      </Panel>
    </UI>)";

  Layout layout;
  auto result = XamlLoader().LoadInto(layout, source);
  ASSERT_TRUE(result) << result.Error;
  auto *panel = dynamic_cast<PanelNode *>(layout.Find("tools"));
  ASSERT_NE(panel, nullptr);
  EXPECT_EQ(panel->TitleNode->Text, "Tools & Scene");
  EXPECT_EQ(panel->ScrollAreaNode->ContentNode->Children.size(), 1U);
  const auto *drag = panel->GetBehavior<DragBehavior>();
  const auto *scroll = panel->ScrollAreaNode->GetScrollBehavior();
  ASSERT_NE(drag, nullptr);
  ASSERT_NE(scroll, nullptr);
  EXPECT_EQ(drag->Properties.Region, DragRegion::Anywhere);
  EXPECT_TRUE(scroll->Properties.Horizontal);
  EXPECT_EQ(scroll->Properties.HorizontalScrollBar, ScrollBarVisibility::Auto);
  EXPECT_EQ(scroll->Properties.VerticalScrollBar, ScrollBarVisibility::Visible);
  EXPECT_NE(layout.Find("content"), nullptr);
}

TEST(XamlLoaderTest, ReportsInvalidMarkupInEnglish) {
  XamlLoader loader;
  auto unknown = loader.Load("<Unknown />");
  EXPECT_FALSE(unknown);
  EXPECT_NE(unknown.Error.find("Unknown control type"), std::string::npos);

  auto mismatch = loader.Load("<UI><Rect></UI>");
  EXPECT_FALSE(mismatch);
  EXPECT_GT(mismatch.ErrorOffset, 0U);
  EXPECT_NE(mismatch.Error.find("Mismatched closing tag"), std::string::npos);

  auto duplicate =
      loader.Load("<UI><Rect Id=\"same\"/><Rect Id=\"same\"/></UI>");
  EXPECT_FALSE(duplicate);
  EXPECT_NE(duplicate.Error.find("Duplicate control key"), std::string::npos);
}

TEST(XamlLoaderTest, CreatesSceneViewportNode) {
  Layout layout;
  const auto result =
      XamlLoader().LoadInto(layout, "<UI><Scene Id=\"viewport\" /></UI>");

  ASSERT_TRUE(result) << result.Error;
  ASSERT_NE(layout.GetSceneNode(), nullptr);
  EXPECT_EQ(static_cast<BaseNode *>(layout.GetSceneNode()),
            layout.Find("viewport"));
}

TEST(XamlLoaderTest, CreatesTerminalNode) {
  Layout layout;
  XamlLoader loader;
  const auto result = loader.LoadInto(
      layout, "<UI><Terminal Id=\"output\" Title=\"Messages\" /></UI>");
  ASSERT_TRUE(result) << result.Error;

  auto *terminal = dynamic_cast<TerminalNode *>(layout.Find("output"));
  ASSERT_NE(terminal, nullptr);
  EXPECT_EQ(terminal->TitleNode->Text, "Messages");
}

TEST(XamlLoaderTest, CreatesPanelGroupWithSwitchablePanels) {
  Layout layout;
  const auto result = XamlLoader().LoadInto(
      layout, "<UI><PanelGroup Id=\"editors\"><Panel Title=\"Scene\"/>"
              "<Panel Title=\"Game\"/></PanelGroup></UI>");
  ASSERT_TRUE(result) << result.Error;

  auto *group = dynamic_cast<PanelGroupNode *>(layout.Find("editors"));
  ASSERT_NE(group, nullptr);
  ASSERT_EQ(group->Panels.size(), 2U);
  EXPECT_EQ(group->Tabs[0]->LabelNode->Text, "Scene");
  EXPECT_EQ(group->Tabs[1]->LabelNode->Text, "Game");
  EXPECT_TRUE(group->Panels[0]->Visible);
  EXPECT_FALSE(group->Panels[1]->Visible);
}

TEST(XamlLoaderTest, CreatesBuiltinImageWithRoundedLayout) {
  Layout layout;
  const auto result = XamlLoader().LoadInto(
      layout, "<UI><Image Id=\"add\" Source=\"builtin://icon/plus\" "
              "Tint=\"#2A8BFFFF\" CornerRadius=\"4\"/></UI>");
  ASSERT_TRUE(result) << result.Error;

  auto *image = dynamic_cast<ImageNode *>(layout.Find("add"));
  ASSERT_NE(image, nullptr);
  EXPECT_EQ(image->Icon, UIIcon::Plus);
  EXPECT_FLOAT_EQ(image->UO->GetCornerRadius(), 4.0f);
}

TEST(XamlLoaderTest, CreatesCommonControlsAndNestedTreeItems) {
  constexpr auto source = R"(
    <UI>
      <Button Id="save" Text="Save" Enabled="true" />
      <Toggle Id="visible" Text="Visible" Checked="true" />
      <Slide Id="volume" Min="10" Max="100" Value="55" Step="5" />
      <TextInput Id="name" Text="Slot" Placeholder="Object name" />
      <TreeView Id="outline">
        <TreeItem Id="rootItem" Text="Root" Expanded="false">
          <TreeItem Id="childItem" Text="Child" Selected="true" />
        </TreeItem>
      </TreeView>
    </UI>)";

  Layout layout;
  const auto result = XamlLoader().LoadInto(layout, source);
  ASSERT_TRUE(result) << result.Error;

  auto *button = dynamic_cast<ButtonNode *>(layout.Find("save"));
  auto *toggle = dynamic_cast<ToggleNode *>(layout.Find("visible"));
  auto *slider = dynamic_cast<SliderNode *>(layout.Find("volume"));
  auto *input = dynamic_cast<TextInputNode *>(layout.Find("name"));
  auto *tree = dynamic_cast<TreeViewNode *>(layout.Find("outline"));
  auto *rootItem = dynamic_cast<TreeViewItemNode *>(layout.Find("rootItem"));
  auto *childItem = dynamic_cast<TreeViewItemNode *>(layout.Find("childItem"));
  ASSERT_NE(button, nullptr);
  ASSERT_NE(toggle, nullptr);
  ASSERT_NE(slider, nullptr);
  ASSERT_NE(input, nullptr);
  ASSERT_NE(tree, nullptr);
  ASSERT_NE(rootItem, nullptr);
  ASSERT_NE(childItem, nullptr);
  EXPECT_EQ(button->LabelNode->Text, "Save");
  EXPECT_TRUE(toggle->Checked);
  EXPECT_FLOAT_EQ(slider->Minimum, 10.0f);
  EXPECT_FLOAT_EQ(slider->Maximum, 100.0f);
  EXPECT_FLOAT_EQ(slider->Value, 55.0f);
  EXPECT_EQ(input->Text, "Slot");
  EXPECT_FALSE(rootItem->Expanded);
  EXPECT_EQ(childItem->Parent, rootItem->ItemsNode);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_EQ(tree->SelectedItem, childItem);
}

TEST(XamlLoaderTest, CreatesFileExplorerAsSpecializedTreeView) {
  Layout layout;
  const auto result = XamlLoader().LoadInto(
      layout,
      "<UI><FileExplorer Id=\"files\" RootPath=\"asset\" "
      "ShowHidden=\"false\" /></UI>");
  ASSERT_TRUE(result) << result.Error;

  auto* explorer = dynamic_cast<FileExplorerNode*>(layout.Find("files"));
  ASSERT_NE(explorer, nullptr);
  EXPECT_EQ(explorer->RootPath, std::filesystem::path("asset"));
  EXPECT_FALSE(explorer->ShowHidden);
}

TEST(XamlLoaderTest, CreatesToolBarWithCascadingMenus) {
  constexpr auto source = R"(
    <UI>
      <ToolBar Id="toolbar" Dock="Top" DockExtent="36">
        <Menu Id="file" Text="File">
          <Menu Id="open" Text="Open">
            <Menu Id="recent" Text="Recent">
              <MenuItem Id="demo" Text="Demo.slot" />
            </Menu>
          </Menu>
        </Menu>
      </ToolBar>
      <Scene Id="scene" />
    </UI>)";

  Layout layout;
  const auto result = XamlLoader().LoadInto(layout, source);
  ASSERT_TRUE(result) << result.Error;
  auto *toolbar = dynamic_cast<ToolBarNode *>(layout.Find("toolbar"));
  auto *file = dynamic_cast<MenuNode *>(layout.Find("file"));
  auto *open = dynamic_cast<MenuNode *>(layout.Find("open"));
  auto *recent = dynamic_cast<MenuNode *>(layout.Find("recent"));
  auto *demo = dynamic_cast<MenuItemNode *>(layout.Find("demo"));
  ASSERT_NE(toolbar, nullptr);
  ASSERT_NE(file, nullptr);
  ASSERT_NE(open, nullptr);
  ASSERT_NE(recent, nullptr);
  ASSERT_NE(demo, nullptr);
  EXPECT_TRUE(file->IsTopLevel());
  EXPECT_EQ(open->Parent, file->PopupNode);
  EXPECT_EQ(recent->Parent, open->PopupNode);
  EXPECT_EQ(demo->Parent, recent->PopupNode);
}
} // namespace z8::ui
