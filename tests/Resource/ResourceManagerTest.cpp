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
