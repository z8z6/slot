#include "Core/Scene.h"

#include "Light/ParallelLight.h"
#include "Object/Camera/Camera.h"
#include "Object/GameObject/CubeObject.h"

#include <gtest/gtest.h>

namespace z8 {

TEST(SceneTest, OwnsCameraLightAndGameObjects) {
  Scene scene;
  scene.SetCamera(std::make_unique<Camera>());
  scene.SetLight(std::make_unique<ParallelLight>());
  auto& cube = scene.CreateGameObject<CubeObject>();

  EXPECT_NE(scene.GetCamera(), nullptr);
  EXPECT_NE(scene.GetLight(), nullptr);
  ASSERT_EQ(scene.GetGameObjects().size(), 1U);
  EXPECT_EQ(scene.GetGameObjects().front(), &cube);
}

} // namespace z8
