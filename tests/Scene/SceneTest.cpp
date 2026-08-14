#include "Core/Scene.h"

#include "Light/BaseLight.h"
#include "Object/Camera/BaseCamera.h"
#include "Object/GameObject/CubeObject.h"

#include <gtest/gtest.h>

namespace z8 {

TEST(SceneTest, OwnsDefaultContextAndGameObjects) {
  Scene scene;
  auto* cube = scene.GOs.add<CubeObject>();

  EXPECT_NE(scene.Camera.get(), nullptr);
  EXPECT_NE(scene.Light.get(), nullptr);
  ASSERT_EQ(scene.GOs.size(), 1U);
  EXPECT_EQ(&scene.GOs.front(), cube);
}

} // namespace z8
