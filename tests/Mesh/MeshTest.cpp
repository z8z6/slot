#include "Mesh/CubeMesh.h"
#include "Mesh/GridMesh.h"
#include "Mesh/RectMesh.h"
#include "Mesh/SphereMesh.h"

#include <DirectXMath.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>

namespace z8 {

TEST(MeshTest, SphereSubdivisionStaysWithinSixteenBitIndices) {
  SphereMesh sphere(2.0f, 6);

  EXPECT_GE(sphere.V.size(), 40962U);
  EXPECT_LT(sphere.V.size(), 65536U);
  EXPECT_EQ(sphere.I.size(), 245760U);
  EXPECT_TRUE(sphere.Validate());
  EXPECT_LT(*std::max_element(sphere.I.begin(), sphere.I.end()),
            sphere.V.size());
  EXPECT_EQ(sphere.NormalMode, MeshNormalMode::PreserveAuthored);
  for (const auto& vertex : sphere.V) {
    const float positionLength = std::sqrt(
        vertex.Pos.x * vertex.Pos.x + vertex.Pos.y * vertex.Pos.y +
        vertex.Pos.z * vertex.Pos.z);
    const float normalLength = std::sqrt(
        vertex.Normal.x * vertex.Normal.x + vertex.Normal.y * vertex.Normal.y +
        vertex.Normal.z * vertex.Normal.z);
    EXPECT_NEAR(positionLength, 2.0f, 1.0e-4f);
    EXPECT_NEAR(normalLength, 1.0f, 1.0e-4f);
  }
  for (size_t triangle = 0; triangle < sphere.I.size(); triangle += 3) {
    const float u0 = sphere.V[sphere.I[triangle]].TexCoord.x;
    const float u1 = sphere.V[sphere.I[triangle + 1]].TexCoord.x;
    const float u2 = sphere.V[sphere.I[triangle + 2]].TexCoord.x;
    EXPECT_LE(std::max({u0, u1, u2}) - std::min({u0, u1, u2}), 0.5001f);
  }
}

TEST(MeshTest, GridHasUpwardNormalsAndCompleteTextureCoordinates) {
  GridMesh grid(4.0f, 2.0f, 3, 4);
  ASSERT_TRUE(grid.Validate());
  grid.ComputeNormals();

  EXPECT_EQ(grid.V.size(), 12U);
  EXPECT_FLOAT_EQ(grid.V.front().TexCoord.x, 0.0f);
  EXPECT_FLOAT_EQ(grid.V.front().TexCoord.y, 0.0f);
  EXPECT_FLOAT_EQ(grid.V.back().TexCoord.x, 1.0f);
  EXPECT_FLOAT_EQ(grid.V.back().TexCoord.y, 1.0f);
  for (const auto& vertex : grid.V) {
    EXPECT_NEAR(vertex.Normal.x, 0.0f, 1.0e-6f);
    EXPECT_NEAR(vertex.Normal.y, 1.0f, 1.0e-6f);
    EXPECT_NEAR(vertex.Normal.z, 0.0f, 1.0e-6f);
  }
}

TEST(MeshTest, HardEdgedAndDoubleSidedMeshesDoNotShareFaceVertices) {
  CubeMesh cube;
  RectMesh rect;
  EXPECT_EQ(cube.V.size(), 24U);
  EXPECT_EQ(rect.V.size(), 8U);
  ASSERT_TRUE(cube.Validate());
  ASSERT_TRUE(rect.Validate());
  cube.ComputeNormals();
  rect.ComputeNormals();
  EXPECT_LT(rect.V[0].Normal.z, -0.99f);
  EXPECT_GT(rect.V[4].Normal.z, 0.99f);
}

TEST(MeshTest, ValidationRejectsOutOfRangeIndices) {
  Mesh mesh;
  mesh.V.resize(3);
  mesh.I = {0, 1, 3};
  std::string error;
  EXPECT_FALSE(mesh.Validate(&error));
  EXPECT_EQ(error, "Mesh contains an out-of-range index.");
}

} // namespace z8
