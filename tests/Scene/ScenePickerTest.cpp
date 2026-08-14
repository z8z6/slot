#include "Core/ScenePicker.h"

#include "Core/Scene.h"
#include "Object/Camera/Camera.h"
#include "Object/GameObject/CubeObject.h"
#include "Resource/ResourceManager.h"

#include <gtest/gtest.h>

#include <filesystem>

namespace z8 {

TEST(ScenePickerTest, ReturnsNearestTriangleHitInsideViewport) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  ResourceManager resources;
  Scene scene;
  Camera camera;
  camera.UpdateProj(1.0f);
  camera.Update(nullptr);

  auto& nearCube = scene.CreateGameObject<CubeObject>();
  nearCube.Transform.Scale = {2.0f, 2.0f, 2.0f};
  auto& farCube = scene.CreateGameObject<CubeObject>();
  farCube.Transform.Position.z = 10.0f;
  farCube.Transform.Scale = {2.0f, 2.0f, 2.0f};

  const ScenePickRect viewport{0.0f, 0.0f, 200.0f, 200.0f};
  EXPECT_EQ(ScenePicker::Pick(scene, resources, camera, viewport, 100.0f,
                              100.0f),
            &nearCube);
  EXPECT_EQ(ScenePicker::Pick(scene, resources, camera, viewport, -1.0f,
                              100.0f),
            nullptr);
}

} // namespace z8
