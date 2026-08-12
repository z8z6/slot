#include "UI/Behavior/DragBehavior.h"
#include "UI/Behavior/ResizeBehavior.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/RectNode.h"

#include "yoga/YGNodeStyle.h"

#include <gtest/gtest.h>

namespace z8::ui {
namespace {

/** 构造不依赖 Win32 消息的测试指针事件，保持坐标和增量语义清晰。 */
MouseMovArgs PointerArgs(int x, int y, int deltaX = 0, int deltaY = 0) {
  MouseMovArgs args;
  args.X = x;
  args.Y = y;
  args.DeltaX = deltaX;
  args.DeltaY = deltaY;
  args.State = MK_LBUTTON;
  args.Button = MouseButton::Left;
  return args;
}

} // namespace

TEST(BehaviorCompositionTest, AddsCapabilitiesWithoutSubclassingControl) {
  Layout layout(nullptr);
  auto rect = std::make_unique<RectNode>();
  auto *observer = rect.get();
  auto *drag = rect->AddBehavior<DragBehavior>();
  auto *resize = rect->AddBehavior<ResizeBehavior>();
  DragProperty dragProperties;
  dragProperties.Region = DragRegion::Anywhere;
  drag->Properties = dragProperties;

  YGNodeStyleSetWidth(rect->Node, 200.0f);
  YGNodeStyleSetHeight(rect->Node, 120.0f);
  YGNodeStyleSetFlexGrow(rect->Node, 0.0f);
  YGNodeStyleSetFlexShrink(rect->Node, 0.0f);
  YGNodeStyleSetMargin(rect->Node, YGEdgeAll, 0.0f);
  layout.Root->AddChild(std::move(rect));
  layout.RebuildIndex();
  layout.Calculate(800.0f, 600.0f);

  ASSERT_EQ(observer->GetBehavior<DragBehavior>(), drag);
  ASSERT_EQ(observer->GetBehavior<ResizeBehavior>(), resize);
  ASSERT_EQ(observer->Behaviors.front().get(),
            static_cast<IBehavior *>(resize));

  // 角落同时命中 Drag 和 Resize；优先级使 Resize 独占捕获，不会移动左上角。
  ASSERT_NE(layout.OnMouseDown(PointerArgs(199, 119)), EventReply::Ignored);
  EXPECT_TRUE(resize->IsResizing());
  EXPECT_FALSE(drag->IsDragging());
  ASSERT_NE(layout.OnMouseDrag(PointerArgs(219, 129, 20, 10)),
            EventReply::Ignored);
  ASSERT_NE(layout.OnMouseUp(PointerArgs(219, 129)), EventReply::Ignored);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(observer->Width, 220.0f);
  EXPECT_FLOAT_EQ(observer->Height, 130.0f);

  // 属性链也由挂载关系组成，无需给 RectNode 增加 Drag 专用分支。
  EXPECT_TRUE(observer->SetProperty("Draggable", "false"));
  EXPECT_FALSE(drag->Properties.Enabled);
  EXPECT_TRUE(observer->RemoveBehavior(drag));
  EXPECT_EQ(observer->GetBehavior<DragBehavior>(), nullptr);
}

TEST(BehaviorCompositionTest, CancelsGestureWhenTopologyChanges) {
  Layout layout(nullptr);
  auto rect = std::make_unique<RectNode>();
  auto *observer = rect.get();
  auto *drag = rect->AddBehavior<DragBehavior>();
  DragProperty properties;
  properties.Region = DragRegion::Anywhere;
  drag->Properties = properties;
  YGNodeStyleSetWidth(rect->Node, 100.0f);
  YGNodeStyleSetHeight(rect->Node, 100.0f);
  YGNodeStyleSetMargin(rect->Node, YGEdgeAll, 0.0f);
  layout.Root->AddChild(std::move(rect));
  layout.RebuildIndex();
  layout.Calculate(400.0f, 300.0f);

  ASSERT_NE(layout.OnMouseDown(PointerArgs(50, 50)), EventReply::Ignored);
  ASSERT_TRUE(drag->IsDragging());
  // 声明式重建会使旧 target 指针失效，因此必须同步取消 Behavior 内部状态。
  layout.RebuildIndex();
  EXPECT_FALSE(drag->IsDragging());
  EXPECT_EQ(observer->GetBehavior<DragBehavior>(), drag);

  ASSERT_NE(layout.OnMouseDown(PointerArgs(50, 50)), EventReply::Ignored);
  layout.Root->RemoveChildrenFrom(0);
  // 被移除节点的析构已取消其行为；重建只遍历当前活节点，不解引用旧捕获指针。
  layout.RebuildIndex();
  EXPECT_EQ(layout.OnMouseDrag(PointerArgs(60, 60, 10, 10)),
            EventReply::Ignored);
}

} // namespace z8::ui
