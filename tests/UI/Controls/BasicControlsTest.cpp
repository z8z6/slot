#include "UI/Layout/ButtonNode.h"
#include "UI/Layout/Layout.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(BasicControlsTest, ButtonPublishesPointerActivation) {
  Layout layout;
  auto button = std::make_unique<ButtonNode>();
  auto* observer = button.get();
  button->Style.Width = 200.0f;
  layout.Root->AddChild(std::move(button));
  layout.RebuildIndex();
  layout.Calculate(320.0f, 120.0f);

  MouseMovArgs pointer;
  pointer.X = static_cast<int>(observer->Left + observer->Width * 0.5f);
  pointer.Y = static_cast<int>(observer->Top + observer->Height * 0.5f);
  pointer.State = MK_LBUTTON;
  pointer.Button = MouseButton::Left;
  EXPECT_EQ(layout.OnMouseDown(pointer), EventReply::Handled);
  EXPECT_EQ(layout.OnMouseUp(pointer), EventReply::Handled);
  EXPECT_TRUE(observer->ConsumeClicked());
}

} // namespace z8::ui
