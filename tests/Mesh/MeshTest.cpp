#include "Mesh/BuiltinMesh.h"

#include <gtest/gtest.h>

namespace z8 {

TEST(MeshTest, BuiltinMeshesHaveValidRenderableTopology) {
  CubeMesh cube;
  GridMesh grid;
  RectMesh rect;
  SphereMesh sphere;

  EXPECT_TRUE(cube.Validate());
  EXPECT_TRUE(grid.Validate());
  EXPECT_TRUE(rect.Validate());
  EXPECT_TRUE(sphere.Validate());
  EXPECT_EQ(sphere.NormalMode, MeshNormalMode::PreserveAuthored);
}

} // namespace z8
