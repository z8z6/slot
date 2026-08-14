#include "UI/Layout/ImageNode.h"
#include "Object/UIObject/UIObject.h"
#include "UI/Declarative/ControlFactory.h"
#include "UI/Style/Theme.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(ImageNodeTest, SelectsBuiltinIconAndTint) {
  ImageNode image;

  EXPECT_TRUE(image.SetProperty("Source", "builtin://icon/plus"));
  EXPECT_TRUE(image.SetProperty("Tint", "#2A8BFFFF"));
  EXPECT_EQ(image.Icon, UIIcon::Plus);
  EXPECT_EQ(image.Source, "builtin://icon/plus");
  EXPECT_FLOAT_EQ(image.UO->GetColor().x, 42.0f / 255.0f);
  EXPECT_FALSE(image.SetProperty("Source", "file://missing.png"));
}

TEST(ImageNodeTest, SelectsDownloadedLucideAsset) {
  ImageNode image;

  EXPECT_TRUE(
      image.SetProperty("Source", "asset://texture/icons/lucide/terminal.svg"));
  EXPECT_EQ(image.Icon, UIIcon::Terminal);
  EXPECT_EQ(image.Source, "asset://texture/icons/lucide/terminal.svg");
}

TEST(ImageNodeTest, SelectsSemanticIconWithoutExposingResourcePathToCaller) {
  ImageNode image;

  ASSERT_TRUE(image.SetIcon(UIIcon::Close));
  EXPECT_EQ(image.Icon, UIIcon::Close);
  EXPECT_EQ(image.Source, GetUIIconInfo(UIIcon::Close).Source);
  EXPECT_FLOAT_EQ(image.Style.Width.value(), Theme::Default().Icon.NormalSize);
}

TEST(ImageNodeTest, IsAvailableToDeclarativeConstruction) {
  auto node = ControlFactory::Instance().Create("Image");
  EXPECT_NE(dynamic_cast<ImageNode *>(node.get()), nullptr);
}

} // namespace z8::ui
