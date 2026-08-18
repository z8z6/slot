#include "Object/Camera/FirstPersonCamera.h"

#include <DirectXMath.h>
#include <cmath>
#include <gtest/gtest.h>

namespace z8 {

TEST(BaseCameraTest, UsesModeratePerspectiveWithoutDepthStretching) {
  BaseCamera camera;
  constexpr float aspect = 16.0f / 9.0f;
  camera.UpdateProj(aspect);

  const auto &projection = camera.GetProj();
  // 60° 垂直 FOV 的缩放项为 cot(30°)=sqrt(3)；同时校验宽高比只作用于
  // 水平项，防止视口调整再次把几何比例误当成世界空间缩放。
  EXPECT_NEAR(projection._22, std::sqrt(3.0f), 1.0e-5f);
  EXPECT_NEAR(projection._11 * aspect, projection._22, 1.0e-5f);
}

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
