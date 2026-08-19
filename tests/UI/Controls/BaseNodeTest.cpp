#include "UI/Layout/BaseNode.h"
#include "../../../include/Object/UIObject.h"
#include "UI/Layout/DrawNode.h"

#include <gtest/gtest.h>

namespace z8::ui {
namespace {

class TrackingObject final : public UIObject {
public:
  ~TrackingObject() override { ++DestructionCount; }
  static inline int DestructionCount = 0;
};

} // namespace

TEST(BaseNodeTest, OwnsVisualAndChildLifetime) {
  TrackingObject::DestructionCount = 0;
  {
    BaseNode root;
    auto child = std::make_unique<DrawNode>(
        std::make_unique<TrackingObject>());
    auto* observer = child.get();
    EXPECT_EQ(root.AddChild(std::move(child)), observer);
    EXPECT_EQ(observer->Parent, &root);
  }
  EXPECT_EQ(TrackingObject::DestructionCount, 1);
}

} // namespace z8::ui
