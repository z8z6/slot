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
      resources.Resolve(ResourceRef<Mesh>(builtin::CubeMesh));
  const auto material =
      resources.Resolve(ResourceRef<Material>(builtin::MetalMaterial));
  const auto program = resources.Resolve(
      ResourceRef<ShaderProgram>(builtin::GameObjectProgram));

  EXPECT_NE(resources.TryGet(mesh), nullptr);
  ASSERT_NE(resources.TryGet(material), nullptr);
  EXPECT_EQ(resources.Resolve(resources.TryGet(material)->Program), program);
  ASSERT_NE(resources.TryGet(program), nullptr);
  EXPECT_TRUE(resources.TryGet(program)->VertexShader.IsValid());
  EXPECT_TRUE(resources.TryGet(program)->PixelShader.IsValid());

  static_assert(!std::is_same_v<ResourceHandle<Mesh>,
                                ResourceHandle<Material>>);
}

TEST(ResourceManagerTest, AddsResourceUsingItsOwnNameAndClass) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  ResourceManager resources;

  auto shader = std::make_unique<Shader>();
  shader->AssetId = "builtin://shader/test/vertex";
  shader->Name = "Test_V";
  const auto handle = resources.Add(std::move(shader));

  ASSERT_TRUE(handle.IsValid());
  EXPECT_EQ(resources.TryGet(handle)->GetName(),
            "builtin://shader/test/vertex");
  EXPECT_EQ(resources.Resolve(
                ResourceRef<Shader>("builtin://shader/test/vertex")),
            handle);
}

} // namespace z8
