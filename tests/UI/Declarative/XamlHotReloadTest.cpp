#include "UI/Declarative/XamlHotReload.h"

#include "UI/Layout/Layout.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Layout/PanelNode.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace z8::ui {
namespace {

class TemporaryXamlFile final {
public:
  std::filesystem::path Path =
      std::filesystem::temp_directory_path() /
      "slot-xaml-hot-reload-test.xaml";

  ~TemporaryXamlFile() {
    std::error_code error;
    std::filesystem::remove(Path, error);
  }

  void Write(const std::string &source) const {
    std::ofstream output(Path, std::ios::binary | std::ios::trunc);
    output << source;
  }
};

} // namespace

TEST(XamlHotReloadTest, ReloadsChangedFileAndPreservesLastValidTree) {
  TemporaryXamlFile file;
  file.Write("<UI><Panel Id=\"panel\" Title=\"First\"/></UI>");
  Layout layout;
  XamlHotReload reload(file.Path);

  ASSERT_EQ(reload.Poll(layout), XamlReloadStatus::Reloaded);
  auto *first = dynamic_cast<PanelNode *>(layout.Find("panel"));
  ASSERT_NE(first, nullptr);
  EXPECT_EQ(first->TitleNode->Text, "First");
  EXPECT_EQ(reload.Poll(layout), XamlReloadStatus::Unchanged);

  file.Write("<UI><Panel Id=\"panel\" Title=\"Second title\"/></UI>");
  ASSERT_EQ(reload.Poll(layout), XamlReloadStatus::Reloaded);
  auto *second = dynamic_cast<PanelNode *>(layout.Find("panel"));
  ASSERT_NE(second, nullptr);
  EXPECT_EQ(second->TitleNode->Text, "Second title");

  file.Write("<UI><Panel Id=\"broken\"></UI>");
  EXPECT_EQ(reload.Poll(layout), XamlReloadStatus::Failed);
  EXPECT_EQ(layout.Find("panel"), second);
  EXPECT_NE(reload.GetLastError().find("Mismatched closing tag"),
            std::string::npos);
}

TEST(XamlHotReloadTest, LoadsProjectInterfaceFromAssetXml) {
  Layout layout;
  XamlHotReload reload(std::filesystem::path(SLOT_SOURCE_DIR) /
                       "asset/xml/Main.xaml");

  ASSERT_EQ(reload.Poll(layout), XamlReloadStatus::Reloaded)
      << reload.GetLastError();
  EXPECT_NE(layout.Find("toolbar"), nullptr);
  EXPECT_NE(layout.Find("terminal"), nullptr);
  EXPECT_NE(layout.Find("outliner"), nullptr);
  EXPECT_NE(layout.Find("details"), nullptr);
  EXPECT_NE(layout.Find("scene-viewport"), nullptr);
  layout.Calculate(1600.0f, 900.0f);
  const auto *toolbar = dynamic_cast<PanelNode *>(layout.Find("toolbar"));
  const auto *terminal = dynamic_cast<PanelNode *>(layout.Find("terminal"));
  const auto *outliner = dynamic_cast<PanelNode *>(layout.Find("outliner"));
  const auto *details = dynamic_cast<PanelNode *>(layout.Find("details"));
  ASSERT_NE(toolbar, nullptr);
  ASSERT_NE(terminal, nullptr);
  ASSERT_NE(outliner, nullptr);
  ASSERT_NE(details, nullptr);
  ASSERT_NE(toolbar->Group, nullptr);
  ASSERT_NE(terminal->Group, nullptr);
  ASSERT_NE(outliner->Group, nullptr);
  ASSERT_NE(details->Group, nullptr);
  EXPECT_TRUE(layout.Dock.IsDocked(*toolbar->Group));
  EXPECT_TRUE(layout.Dock.IsDocked(*terminal->Group));
  EXPECT_NEAR(toolbar->Group->Height, 48.0f, 0.01f);
  EXPECT_NEAR(terminal->Group->Height, 180.0f, 0.01f);
  EXPECT_NEAR(outliner->Group->Width, 260.0f, 0.01f);
  EXPECT_NEAR(details->Group->Width, 300.0f, 0.01f);
  EXPECT_TRUE(layout.Dock.Validate());
}

} // namespace z8::ui
