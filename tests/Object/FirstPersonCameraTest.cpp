#include "Object/Camera/FirstPersonCamera.h"

#include <gtest/gtest.h>

namespace z8 {

TEST(FirstPersonCameraTest, IgnoresMouseMotionByDefault) {
  FirstPersonCamera camera;
  const auto before = camera.Transform.Rotation;
  MouseMovArgs movement;
  movement.DeltaX = 40;
  movement.DeltaY = -20;

  EXPECT_EQ(camera.OnMouseMove(movement), EventReply::Ignored);
  EXPECT_FLOAT_EQ(camera.Transform.Rotation.x, before.x);
  EXPECT_FLOAT_EQ(camera.Transform.Rotation.y, before.y);
}

} // namespace z8
