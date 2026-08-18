#include "Mesh/FbxImporter.h"

#include <gtest/gtest.h>

#include <string_view>

namespace z8 {

TEST(FbxImporterTest, ImportsRenderableAsciiMesh) {
  constexpr std::string_view source = R"fbx(
Objects: {
  Geometry: 1, "Geometry::Quad", "Mesh" {
    Vertices: *12 { a: -1,-1,1, 1,-1,1, 1,1,1, -1,1,1 }
    PolygonVertexIndex: *4 { a: 0,1,2,-4 }
    LayerElementNormal: 0 {
      MappingInformationType: "ByPolygonVertex"
      ReferenceInformationType: "Direct"
      Normals: *12 { a: 0,0,1, 0,0,1, 0,0,1, 0,0,1 }
    }
  }
}
)fbx";

  // 通过当前统一导入器入口验证内存文本路径，避免测试依赖临时文件。
  auto result = FbxImporter::ParseText(source);

  ASSERT_TRUE(result) << result.Error;
  ASSERT_NE(result.Value, nullptr);
  EXPECT_EQ(result.Value->V.size(), 4U);
  EXPECT_EQ(result.Value->I.size(), 6U);
  EXPECT_TRUE(result.Value->Validate());
}

} // namespace z8
