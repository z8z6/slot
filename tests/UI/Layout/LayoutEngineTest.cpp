#include "UI/Layout/BaseNode.h"
#include "UI/Layout/LayoutEngine.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(LayoutEngineTest, DistributesRowSpaceByGrowFactor) {
  BaseNode root;
  root.Style.Direction = FlexDirection::Row;
  auto first = std::make_unique<BaseNode>();
  auto second = std::make_unique<BaseNode>();
  auto *firstObserver = first.get();
  auto *secondObserver = second.get();
  first->Style.FlexGrow = 1.0f;
  second->Style.FlexGrow = 3.0f;
  root.AddChild(std::move(first));
  root.AddChild(std::move(second));

  LayoutEngine::Calculate(root, 400.0f, 100.0f);

  EXPECT_FLOAT_EQ(firstObserver->Computed.Width, 100.0f);
  EXPECT_FLOAT_EQ(secondObserver->Computed.Left, 100.0f);
  EXPECT_FLOAT_EQ(secondObserver->Computed.Width, 300.0f);
}

TEST(LayoutEngineTest, ResolvesAbsoluteOpposingEdgesInsidePadding) {
  BaseNode root;
  root.Style.Padding = 10.0f;
  auto child = std::make_unique<BaseNode>();
  auto *observer = child.get();
  child->Style.Position = PositionType::Absolute;
  child->Style.Left = 5.0f;
  child->Style.Right = 15.0f;
  child->Style.Top = 20.0f;
  child->Style.Bottom = 30.0f;
  root.AddChild(std::move(child));

  LayoutEngine::Calculate(root, 200.0f, 150.0f);

  EXPECT_FLOAT_EQ(observer->Computed.Left, 15.0f);
  EXPECT_FLOAT_EQ(observer->Computed.Top, 30.0f);
  EXPECT_FLOAT_EQ(observer->Computed.Width, 160.0f);
  EXPECT_FLOAT_EQ(observer->Computed.Height, 80.0f);
}

TEST(LayoutEngineTest, ShrinksFlowWithoutViolatingMinimum) {
  BaseNode root;
  auto first = std::make_unique<BaseNode>();
  auto second = std::make_unique<BaseNode>();
  auto *firstObserver = first.get();
  auto *secondObserver = second.get();
  first->Style.Height = 100.0f;
  first->Style.MinHeight = 80.0f;
  second->Style.Height = 100.0f;
  second->Style.MinHeight = 20.0f;
  root.AddChild(std::move(first));
  root.AddChild(std::move(second));

  LayoutEngine::Calculate(root, 100.0f, 100.0f);

  EXPECT_FLOAT_EQ(firstObserver->Computed.Height, 80.0f);
  EXPECT_FLOAT_EQ(secondObserver->Computed.Height, 20.0f);
  EXPECT_FLOAT_EQ(secondObserver->Computed.Top, 80.0f);
}

} // namespace z8::ui
