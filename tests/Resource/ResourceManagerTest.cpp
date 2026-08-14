#include "Resource/BuiltinResource.h"
#include "Resource/ResourceManager.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <type_traits>

namespace z8 {

TEST(ResourceManagerTest, ResolvesTypedBuiltinReferences) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  ResourceManager resources;

  const auto mesh =
      resources.Resolve(ResourceReference<Mesh>(builtin::CubeMesh));
  const auto material =
      resources.Resolve(ResourceReference<Material>(builtin::MetalMaterial));
  const auto program = resources.Resolve(
      ResourceReference<ShaderProgram>(builtin::GameObjectProgram));

  EXPECT_NE(resources.TryGet(mesh), nullptr);
  ASSERT_NE(resources.TryGet(material), nullptr);
  // 默认金属材质必须提供足够的镜面反射率和有限粗糙度，否则高光项虽存在但
  // 在常见观察角度下会数值性消失。
  EXPECT_GT(resources.TryGet(material)->FresnelR0.x, 0.1f);
  EXPECT_GT(resources.TryGet(material)->Rough, 0.0f);
  EXPECT_LT(resources.TryGet(material)->Rough, 1.0f);
  EXPECT_EQ(resources.Resolve(resources.TryGet(material)->Program), program);
  ASSERT_NE(resources.TryGet(program), nullptr);
  EXPECT_TRUE(resources.TryGet(program)->VertexShader.IsValid());
  EXPECT_TRUE(resources.TryGet(program)->PixelShader.IsValid());

  static_assert(!std::is_same_v<ResourceHandle<Mesh>,
                                ResourceHandle<Material>>);
}

TEST(ResourceManagerTest, RejectsUnknownAssetReference) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  ResourceManager resources;
  const auto missing =
      resources.Resolve(ResourceReference<Mesh>("missing://mesh"));
  EXPECT_FALSE(missing.IsValid());
  EXPECT_EQ(resources.TryGet(missing), nullptr);
}

} // namespace z8
