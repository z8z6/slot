#include "UI/Layout/ButtonNode.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/SliderNode.h"
#include "UI/Layout/TextInputNode.h"
#include "UI/Layout/ToggleNode.h"
#include "Object/UIObject/UIObject.h"
#include "UI/Style/Theme.h"

#include <gtest/gtest.h>

namespace z8::ui {
namespace {

MouseMovArgs LeftPointer(const BaseNode &node, float ratio = 0.5f) {
  MouseMovArgs args;
  args.State = MK_LBUTTON;
  args.Button = MouseButton::Left;
  args.X = static_cast<int>(node.Left + node.Width * ratio);
  args.Y = static_cast<int>(node.Top + node.Height * 0.5f);
  return args;
}

template <typename Control>
Control *AddControl(Layout &layout, std::unique_ptr<Control> control) {
  auto *observer = control.get();
  control->Style.Width = 200.0f;
  layout.Root->AddChild(std::move(control));
  layout.RebuildIndex();
  layout.Calculate(320.0f, 120.0f);
  return observer;
}

} // namespace

TEST(BasicControlsTest, ButtonActivatesByPointerAndKeyboard) {
  Layout layout;
  auto button = std::make_unique<ButtonNode>();
  button->SetText("Apply");
  auto *observer = AddControl(layout, std::move(button));
  auto pointer = LeftPointer(*observer);

  EXPECT_EQ(layout.OnMouseDown(pointer), EventReply::Handled);
  EXPECT_EQ(layout.CapturedHandler, observer);
  EXPECT_EQ(layout.OnMouseUp(pointer), EventReply::Handled);
  EXPECT_TRUE(observer->ConsumeClicked());
  EXPECT_FALSE(observer->ConsumeClicked());
  EXPECT_TRUE(observer->Focused);

  EXPECT_EQ(layout.OnKeyDown(KeyArgs(VK_RETURN)), EventReply::Handled);
  EXPECT_TRUE(observer->ConsumeClicked());
}

TEST(BasicControlsTest, ToggleOwnsBooleanStateAndCanBeDisabled) {
  Layout layout;
  auto toggle = std::make_unique<ToggleNode>();
  toggle->SetText("Visible");
  auto *observer = AddControl(layout, std::move(toggle));
  auto pointer = LeftPointer(*observer);

  layout.OnMouseDown(pointer);
  layout.OnMouseUp(pointer);
  EXPECT_TRUE(observer->Checked);
  EXPECT_TRUE(observer->ConsumeChanged());

  observer->SetEnabled(false);
  layout.OnMouseDown(pointer);
  layout.OnMouseUp(pointer);
  EXPECT_TRUE(observer->Checked);
  EXPECT_FALSE(observer->ConsumeChanged());
}

TEST(BasicControlsTest, ToggleFocusDoesNotLookSelectedAfterPointerLeaves) {
  Layout layout;
  auto toggle = std::make_unique<ToggleNode>();
  auto *observer = AddControl(layout, std::move(toggle));
  auto pointer = LeftPointer(*observer);

  // 从选中状态点击为未选中，复现控件仍有键盘焦点但鼠标已经离开的路径。
  observer->SetChecked(true, false);
  layout.OnMouseDown(pointer);
  layout.OnMouseUp(pointer);
  ASSERT_FALSE(observer->Checked);
  ASSERT_TRUE(observer->Focused);

  MouseMovArgs outside;
  outside.X = 310;
  outside.Y = 110;
  layout.OnMouseMove(outside);
  ASSERT_FALSE(observer->Hovered);
  const auto &style = Theme::Default().Toggle;
  EXPECT_FLOAT_EQ(observer->IndicatorNode->UO->GetColor().x,
                  style.IndicatorColor.Normal.x);
  EXPECT_FLOAT_EQ(observer->IndicatorNode->UO->GetBorderColor().x,
                  style.FocusedBorderColor.x);
  EXPECT_NE(observer->IndicatorNode->UO->GetColor().x,
            style.IndicatorColor.Selected.x);
}

TEST(BasicControlsTest, SliderUpdatesImmediatelyDuringCapturedDrag) {
  Layout layout;
  auto slider = std::make_unique<SliderNode>();
  ASSERT_TRUE(slider->SetRange(0.0f, 100.0f));
  slider->Step = 5.0f;
  auto *observer = AddControl(layout, std::move(slider));
  auto pointer = LeftPointer(*observer, 0.25f);

  EXPECT_EQ(layout.OnMouseDown(pointer), EventReply::Handled);
  EXPECT_EQ(layout.CapturedHandler, observer);
  pointer.X = static_cast<int>(observer->Left + observer->Width * 0.75f);
  pointer.DeltaX = static_cast<int>(observer->Width * 0.5f);
  EXPECT_EQ(layout.OnMouseDrag(pointer), EventReply::Handled);
  EXPECT_NEAR(observer->Value, 75.0f, 5.0f);
  EXPECT_TRUE(observer->ConsumeChanged());

  observer->Synchronize();
  EXPECT_GT(observer->FillNode->Computed.Width, observer->Width * 0.5f);
  EXPECT_GT(observer->ThumbNode->Computed.Left, observer->Width * 0.5f);
}

TEST(BasicControlsTest, TextInputRoutesUtf8EditingThroughFocusedControl) {
  Layout layout;
  auto input = std::make_unique<TextInputNode>();
  input->SetPlaceholder("Name");
  auto *observer = AddControl(layout, std::move(input));
  auto pointer = LeftPointer(*observer);

  layout.OnMouseDown(pointer);
  layout.OnMouseUp(pointer);
  ASSERT_TRUE(observer->Focused);
  EXPECT_EQ(layout.OnTextInput(L'A'), EventReply::Handled);
  EXPECT_EQ(layout.OnTextInput(L'\x4F60'), EventReply::Handled);
  EXPECT_EQ(observer->Text, "A\xE4\xBD\xA0");
  EXPECT_TRUE(observer->ConsumeChanged());

  EXPECT_EQ(layout.OnKeyDown(KeyArgs(VK_BACK)), EventReply::Handled);
  EXPECT_EQ(observer->Text, "A");
  EXPECT_TRUE(observer->CaretNode->Visible);

  MouseMovArgs outside;
  outside.Button = MouseButton::Left;
  outside.X = 310;
  outside.Y = 110;
  layout.OnMouseDown(outside);
  EXPECT_FALSE(observer->Focused);
  EXPECT_FALSE(observer->CaretNode->Visible);
}

} // namespace z8::ui
