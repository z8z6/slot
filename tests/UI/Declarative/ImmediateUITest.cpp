#include "UI/Declarative/ImmediateUI.h"
#include "Object/UIObject/UIObject.h"
#include "UI/Layout/DrawNode.h"
#include "UI/Layout/ImageNode.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/SceneNode.h"
#include "UI/Layout/TerminalNode.h"
#include "UI/Style/Theme.h"

#include <gtest/gtest.h>

namespace z8::ui {
namespace {
void DeclareFrame(ImmediateUI &ui, bool includeItem) {
  ui.BeginFrame();
  UIStyle panelStyle;
  panelStyle.Width = 320.0f;
  panelStyle.Height = 180.0f;
  if (ui.BeginPanel("panel", "Inspector", panelStyle)) {
    if (includeItem)
      ui.Rect("item", UIStyle{.FlexGrow = 1.0f});
    ui.EndPanel();
  }
  ASSERT_TRUE(ui.EndFrame()) << ui.LastError();
}
} // namespace

TEST(ImmediateUITest, ReusesStableControlsAndTracksTopology) {
  Layout layout;
  layout.ConsumeDirty();
  ImmediateUI ui(layout);

  DeclareFrame(ui, true);
  auto *firstPanel = layout.Find("panel");
  auto *firstItem = layout.Find("item");
  ASSERT_NE(firstPanel, nullptr);
  ASSERT_NE(firstItem, nullptr);
  EXPECT_TRUE(layout.ConsumeDirty());

  DeclareFrame(ui, true);
  EXPECT_EQ(layout.Find("panel"), firstPanel);
  EXPECT_EQ(layout.Find("item"), firstItem);
  EXPECT_FALSE(layout.ConsumeDirty());

  DeclareFrame(ui, false);
  EXPECT_EQ(layout.Find("panel"), firstPanel);
  EXPECT_EQ(layout.Find("item"), nullptr);
  EXPECT_TRUE(layout.ConsumeDirty());
}

TEST(ImmediateUITest, RejectsDuplicateKeysInEnglish) {
  Layout layout;
  ImmediateUI ui(layout);
  ui.BeginFrame();
  ui.Rect("duplicate");
  ui.Rect("duplicate");
  EXPECT_FALSE(ui.EndFrame());
  EXPECT_NE(ui.LastError().find("Duplicate key"), std::string::npos);
}

TEST(ImmediateUITest, RestoresDefaultStyleWhenStyleIsOmitted) {
  Layout layout;
  ImmediateUI ui(layout);

  ui.BeginFrame();
  UIStyle customStyle;
  customStyle.Width = 96.0f;
  customStyle.MinWidth = 72.0f;
  customStyle.FlexGrow = 0.0f;
  customStyle.Margin = 9.0f;
  customStyle.Padding = 7.0f;
  customStyle.Color = DirectX::XMFLOAT4{1.0f, 0.0f, 0.0f, 1.0f};
  customStyle.Direction = FlexDirection::Row;
  auto *first = ui.Rect("item", customStyle);
  ASSERT_NE(first, nullptr);
  ASSERT_TRUE(ui.EndFrame()) << ui.LastError();

  ui.BeginFrame();
  auto *second = ui.Rect("item");
  ASSERT_NE(second, nullptr);
  ASSERT_TRUE(ui.EndFrame()) << ui.LastError();
  ASSERT_EQ(second, first);

  // 空 Style 必须清除上一帧的覆盖，同时仍然复用同一个节点对象。
  const auto &defaults = Theme::Default().Rect;
  EXPECT_FALSE(second->Style.Width.has_value());
  EXPECT_FLOAT_EQ(second->Style.MinWidth, defaults.MinWidth);
  EXPECT_FLOAT_EQ(second->Style.FlexGrow, 1.0f);
  EXPECT_FLOAT_EQ(second->Style.Margin, defaults.Margin);
  EXPECT_FLOAT_EQ(second->Style.Padding, defaults.Padding);
  EXPECT_EQ(second->Style.Direction, FlexDirection::Column);
  auto *drawNode = dynamic_cast<DrawNode *>(second);
  ASSERT_NE(drawNode, nullptr);
  EXPECT_FLOAT_EQ(drawNode->UO->GetColor().x, defaults.Color.x);
}

TEST(ImmediateUITest, DeclaresReusableSceneViewportWithoutStyle) {
  Layout layout;
  ImmediateUI ui(layout);

  ui.BeginFrame();
  auto *first = ui.Scene("viewport");
  ASSERT_NE(first, nullptr);
  ASSERT_TRUE(ui.EndFrame()) << ui.LastError();
  auto *scene = dynamic_cast<SceneNode *>(first);
  ASSERT_NE(scene, nullptr);
  EXPECT_TRUE(scene->ViewportNode->RoutesToScene());
  EXPECT_EQ(layout.GetUO().size(), 1U);

  ui.BeginFrame();
  auto *second = ui.Scene("viewport");
  ASSERT_TRUE(ui.EndFrame()) << ui.LastError();
  EXPECT_EQ(second, first);
  EXPECT_EQ(static_cast<BaseNode *>(layout.GetSceneNode()), first);
}

TEST(ImmediateUITest, DeclaresReusableTerminalWithoutStyle) {
  Layout layout;
  ImmediateUI ui(layout);

  ui.BeginFrame();
  auto *first = ui.Terminal("output", "Messages");
  ASSERT_NE(first, nullptr);
  ASSERT_TRUE(ui.EndFrame()) << ui.LastError();
  auto *terminal = dynamic_cast<TerminalNode *>(first);
  ASSERT_NE(terminal, nullptr);
  EXPECT_EQ(terminal->TitleNode->Text, "Messages");

  terminal->AppendMessage("Persistent message");
  ui.BeginFrame();
  auto *second = ui.Terminal("output", "Messages");
  ASSERT_TRUE(ui.EndFrame()) << ui.LastError();
  EXPECT_EQ(second, first);
  EXPECT_EQ(terminal->OutputText(), "Persistent message");
}

TEST(ImmediateUITest, DeclaresReusableBuiltinImage) {
  Layout layout;
  ImmediateUI ui(layout);
  UIStyle style;
  style.Width = 24.0f;
  style.Height = 24.0f;
  style.CornerRadius = 5.0f;

  ui.BeginFrame();
  auto *first = ui.Image("add", "builtin://icon/plus", style);
  ASSERT_NE(first, nullptr);
  ASSERT_TRUE(ui.EndFrame()) << ui.LastError();
  auto *image = dynamic_cast<ImageNode *>(first);
  ASSERT_NE(image, nullptr);
  EXPECT_EQ(image->Icon, UIIcon::Plus);
  EXPECT_FLOAT_EQ(image->Style.Width.value(), 24.0f);
  EXPECT_FLOAT_EQ(image->UO->GetCornerRadius(), 5.0f);

  ui.BeginFrame();
  auto *second = ui.Image("add", "builtin://icon/close");
  ASSERT_TRUE(ui.EndFrame()) << ui.LastError();
  EXPECT_EQ(second, first);
  EXPECT_EQ(image->Icon, UIIcon::Close);
  EXPECT_FLOAT_EQ(image->Style.Width.value(), Theme::Default().Icon.NormalSize);
  EXPECT_FLOAT_EQ(image->UO->GetColor().x, Theme::Default().Text.MutedColor.x);
}
} // namespace z8::ui
