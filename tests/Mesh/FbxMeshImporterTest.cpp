#include "Mesh/FbxMeshImporter.h"

#include <gtest/gtest.h>

#include <string_view>

namespace z8 {
namespace {

constexpr std::string_view QuadFbx = R"fbx(
; FBX 7.4.0 project file
Objects: {
  Geometry: 1, "Geometry::Quad", "Mesh" {
    Vertices: *12 {
      a: -1,-1,1, 1,-1,1, 1,1,1, -1,1,1
    }
    PolygonVertexIndex: *4 {
      a: 0,1,2,-4
    }
    LayerElementNormal: 0 {
      MappingInformationType: "ByPolygonVertex"
      ReferenceInformationType: "Direct"
      Normals: *12 { a: 0,0,1, 0,0,1, 0,0,1, 0,0,1 }
    }
    LayerElementUV: 0 {
      MappingInformationType: "ByPolygonVertex"
      ReferenceInformationType: "IndexToDirect"
      UV: *8 { a: 0,0, 1,0, 1,1, 0,1 }
      UVIndex: *4 { a: 0,1,2,3 }
    }
  }
}
)fbx";

} // namespace

TEST(FbxMeshImporterTest, ImportsPolygonNormalsAndTextureCoordinates) {
  auto result = FbxMeshImporter::ParseText(QuadFbx);

  ASSERT_TRUE(result) << result.Error;
  ASSERT_NE(result.Value, nullptr);
  EXPECT_EQ(result.Value->Name, "Quad");
  EXPECT_EQ(result.Value->V.size(), 4U);
  EXPECT_EQ(result.Value->I.size(), 6U);
  EXPECT_EQ(result.Value->NormalMode, MeshNormalMode::PreserveAuthored);
  for (const auto& vertex : result.Value->V) {
    EXPECT_NEAR(vertex.Normal.z, -1.0f, 1.0e-6f);
    EXPECT_GE(vertex.TexCoord.x, 0.0f);
    EXPECT_LE(vertex.TexCoord.x, 1.0f);
    EXPECT_GE(vertex.TexCoord.y, 0.0f);
    EXPECT_LE(vertex.TexCoord.y, 1.0f);
  }
}

TEST(FbxMeshImporterTest, GeneratesNormalsWhenLayerIsAbsent) {
  constexpr std::string_view source = R"fbx(
Objects: { Geometry: 2, "Geometry::Triangle", "Mesh" {
Vertices: *9 { a: 0,0,0, 1,0,0, 0,1,0 }
PolygonVertexIndex: *3 { a: 0,1,-3 }
} })fbx";
  auto result = FbxMeshImporter::ParseText(source);

  ASSERT_TRUE(result) << result.Error;
  EXPECT_EQ(result.Value->NormalMode, MeshNormalMode::GenerateSmooth);
  for (const auto& vertex : result.Value->V)
    EXPECT_GT(std::abs(vertex.Normal.z), 0.99f);
}

TEST(FbxMeshImporterTest, AppliesConnectedModelTransform) {
  constexpr std::string_view source = R"fbx(
Objects: {
  Geometry: 10, "Geometry::Triangle", "Mesh" {
    Vertices: *9 { a: 0,0,0, 1,0,0, 0,1,0 }
    PolygonVertexIndex: *3 { a: 0,1,-3 }
  }
  Model: 20, "Model::Triangle", "Mesh" {
    Properties70: {
      P: "Lcl Translation", "Lcl Translation", "", "A",3,4,5
      P: "Lcl Scaling", "Lcl Scaling", "", "A",2,2,2
    }
  }
}
Connections: { C: "OO",10,20
}
)fbx";
  auto result = FbxMeshImporter::ParseText(source);

  ASSERT_TRUE(result) << result.Error;
  ASSERT_EQ(result.Value->V.size(), 3U);
  EXPECT_FLOAT_EQ(result.Value->V[0].Pos.x, 3.0f);
  EXPECT_FLOAT_EQ(result.Value->V[0].Pos.y, 4.0f);
  EXPECT_FLOAT_EQ(result.Value->V[0].Pos.z, -5.0f);
}

TEST(FbxMeshImporterTest, RejectsBinaryFilesWithActionableError) {
  const auto result = FbxMeshImporter::ParseText(
      std::string_view("Kaydara FBX Binary  \x00\x1a", 22));
  EXPECT_FALSE(result);
  EXPECT_NE(result.Error.find("Binary FBX"), std::string::npos);
}

} // namespace z8
