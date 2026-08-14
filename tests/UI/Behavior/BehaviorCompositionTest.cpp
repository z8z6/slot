#include "UI/Behavior/DragBehavior.h"
#include "UI/Behavior/ResizeBehavior.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/RectNode.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(BehaviorCompositionTest, ResolvesCompetingBehaviorsByPriority) {
  Layout layout;
  auto rect = std::make_unique<RectNode>();
  auto* observer = rect.get();
  auto* drag = rect->AddBehavior<DragBehavior>();
  auto* resize = rect->AddBehavior<ResizeBehavior>();
  drag->Properties.Region = DragRegion::Anywhere;
  rect->Style.Width = 200.0f;
  rect->Style.Height = 120.0f;
  rect->Style.FlexGrow = 0.0f;
  rect->Style.FlexShrink = 0.0f;
  rect->Style.Margin = 0.0f;
  layout.Root->AddChild(std::move(rect));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  MouseMovArgs pointer;
  pointer.X = 199;
  pointer.Y = 119;
  pointer.State = MK_LBUTTON;
  pointer.Button = MouseButton::Left;
  ASSERT_EQ(layout.OnMouseDown(pointer), EventReply::Handled);

  EXPECT_TRUE(resize->IsResizing());
  EXPECT_FALSE(drag->IsDragging());
  EXPECT_EQ(observer->GetBehavior<ResizeBehavior>(), resize);
}

} // namespace z8::ui
