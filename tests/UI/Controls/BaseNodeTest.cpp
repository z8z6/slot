#include "UI/Layout/BaseNode.h"
#include "Object/UIObject/UIObject.h"

#include <gtest/gtest.h>

namespace z8::ui {
namespace {
class TrackingObject final : public UIObject {
public:
  ~TrackingObject() override { ++DestructionCount; }
  static inline int DestructionCount = 0;
};

class TrackingNode final : public BaseNode {
public:
  TrackingNode() { SetObject(std::make_unique<TrackingObject>()); }
};

TEST(BaseNodeTest, OwnsVisualAndChildren) {
  TrackingObject::DestructionCount = 0;
  {
    BaseNode root;
    auto child = std::make_unique<TrackingNode>();
    auto* childObserver = child.get();
    EXPECT_EQ(root.AddChild(std::move(child)), childObserver);
    EXPECT_EQ(childObserver->Parent, &root);
    EXPECT_EQ(root.GetChildCount(), 1U);
    EXPECT_EQ(YGNodeGetParent(childObserver->GetYogaNode()), root.GetYogaNode());
  }
  EXPECT_EQ(TrackingObject::DestructionCount, 1);
}

TEST(BaseNodeTest, RemovesOwnedChildSuffix) {
  BaseNode root;
  root.AddChild(std::make_unique<BaseNode>());
  root.AddChild(std::make_unique<BaseNode>());
  root.RemoveChildrenFrom(1);
  EXPECT_EQ(root.GetChildCount(), 1U);
  EXPECT_NE(root.GetChild(0), nullptr);
  EXPECT_EQ(root.GetChild(1), nullptr);
}
} // namespace
} // namespace z8::ui
