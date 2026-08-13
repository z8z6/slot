#include "UI/Layout/BaseNode.h"
#include "Object/Object.h"
#include "Object/UIObject/UIObject.h"
#include "UI/Layout/DrawNode.h"
#include "UI/Layout/BehaviorNode.h"
#include "UI/Layout/Layout.h"

#include <gtest/gtest.h>
#include <stdexcept>
#include <type_traits>

namespace z8::ui {
namespace {
static_assert(!std::is_base_of_v<UIObject, BaseNode>);
static_assert(std::is_base_of_v<BaseNode, DrawNode>);
static_assert(!std::is_base_of_v<EventTarget, BaseNode>);
static_assert(std::is_base_of_v<EventTarget, BehaviorNode>);
static_assert(std::is_base_of_v<EventTarget, Object>);
static_assert(std::is_base_of_v<EventTarget, Layout>);

class TrackingObject final : public UIObject {
public:
  ~TrackingObject() override { ++DestructionCount; }
  static inline int DestructionCount = 0;
};

class TrackingNode final : public DrawNode {
public:
  /** 测试节点通过 DrawNode 验证渲染对象与布局节点的联合生命周期。 */
  TrackingNode() : DrawNode(std::make_unique<TrackingObject>()) {}
};

TEST(BaseNodeTest, OwnsVisualAndChildren) {
  TrackingObject::DestructionCount = 0;
  {
    BaseNode root;
    auto child = std::make_unique<TrackingNode>();
    auto *childObserver = child.get();
    EXPECT_EQ(root.AddChild(std::move(child)), childObserver);
    EXPECT_EQ(childObserver->Parent, &root);
    EXPECT_EQ(root.Children.size(), 1U);
    EXPECT_EQ(childObserver->Parent, &root);
  }
  EXPECT_EQ(TrackingObject::DestructionCount, 1);
}

TEST(BaseNodeTest, RemovesOwnedChildSuffix) {
  BaseNode root;
  root.AddChild(std::make_unique<BaseNode>());
  root.AddChild(std::make_unique<BaseNode>());
  root.RemoveChildrenFrom(1);
  ASSERT_EQ(root.Children.size(), 1U);
  EXPECT_NE(root.Children[0], nullptr);
}

TEST(BaseNodeTest, RejectsDrawNodeWithoutRenderObject) {
  // DrawNode 的类型含义必须可靠，不能允许空视觉重新制造可空渲染分支。
  EXPECT_THROW(DrawNode(nullptr), std::invalid_argument);
}
} // namespace
} // namespace z8::ui
