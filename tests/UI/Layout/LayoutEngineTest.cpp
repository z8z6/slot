#include "UI/Layout/BaseNode.h"
#include "UI/Layout/LayoutEngine.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(LayoutEngineTest, DistributesFlowSpaceByGrowFactor) {
  BaseNode root;
  root.Style.Direction = FlexDirection::Row;
  auto first = std::make_unique<BaseNode>();
  auto second = std::make_unique<BaseNode>();
  auto* firstObserver = first.get();
  auto* secondObserver = second.get();
  first->Style.FlexGrow = 1.0f;
  second->Style.FlexGrow = 3.0f;
  root.AddChild(std::move(first));
  root.AddChild(std::move(second));

  LayoutEngine::Calculate(root, 400.0f, 100.0f);

  EXPECT_FLOAT_EQ(firstObserver->Computed.Width, 100.0f);
  EXPECT_FLOAT_EQ(secondObserver->Computed.Left, 100.0f);
  EXPECT_FLOAT_EQ(secondObserver->Computed.Width, 300.0f);
}

} // namespace z8::ui
