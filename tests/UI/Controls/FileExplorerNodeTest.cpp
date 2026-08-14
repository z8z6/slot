#include "UI/Layout/FileExplorerNode.h"

#include <gtest/gtest.h>

#include <filesystem>

namespace z8::ui {

TEST(FileExplorerNodeTest, BuildsDirectoryFirstTreeAndPublishesSelectedPath) {
  FileExplorerNode explorer;
  const auto rootPath = std::filesystem::path(SLOT_SOURCE_DIR) / "asset";
  ASSERT_TRUE(explorer.SetProperty("RootPath", rootPath.string()));
  ASSERT_EQ(explorer.ContentNode->Children.size(), 1U);

  auto* root = dynamic_cast<TreeViewItemNode*>(
      explorer.ContentNode->Children.front().get());
  ASSERT_NE(root, nullptr);
  ASSERT_NE(explorer.GetPath(root), nullptr);
  EXPECT_EQ(*explorer.GetPath(root), rootPath);
  EXPECT_FALSE(root->ItemsNode->Children.empty());

  std::filesystem::path selected;
  explorer.PathSelected =
      [&](const std::filesystem::path& value) { selected = value; };
  EXPECT_TRUE(explorer.SelectItem(root));
  EXPECT_EQ(selected, rootPath);
}

TEST(FileExplorerNodeTest, RejectsInvalidBooleanProperty) {
  FileExplorerNode explorer;
  EXPECT_FALSE(explorer.SetProperty("ShowHidden", "sometimes"));
}

} // namespace z8::ui
