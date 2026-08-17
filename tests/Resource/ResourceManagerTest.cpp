#include "Resource/ResourceManager.h"
#include "Resource/BuiltinResource.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <type_traits>

namespace z8 {

TEST(ResourceManagerTest, ResolvesTypedBuiltinReferences) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  ResourceManager resources;

  const auto mesh =
      resources.Resolve(ResourceRef<Mesh>(builtin::mesh::CubeMesh));
  const auto material = resources.Resolve(
      ResourceRef<Material>(builtin::material::GrassBlockMaterial));
  const auto program = resources.Resolve(
      ResourceRef<ShaderProgram>(builtin::shader::program::GameObjectProgram));

  EXPECT_NE(resources.TryGet(mesh), nullptr);
  ASSERT_NE(resources.TryGet(material), nullptr);
  EXPECT_EQ(resources.Resolve(resources.TryGet(material)->Program), program);
  const auto texture =
      resources.Resolve(resources.TryGet(material)->BaseColorTexture);
  ASSERT_NE(resources.TryGet(texture), nullptr);
  EXPECT_TRUE(resources.TryGet(texture)->Validate());
  ASSERT_NE(resources.TryGet(program), nullptr);
  EXPECT_TRUE(resources.TryGet(program)->VertexShader.IsValid());
  EXPECT_TRUE(resources.TryGet(program)->PixelShader.IsValid());
  EXPECT_EQ(
      resources.Resolve(ResourceRef<Shader>(builtin::shader::GameObjectVertex)),
      resources.TryGet(program)->VertexShader);
  EXPECT_EQ(
      resources.Resolve(ResourceRef<Shader>(builtin::shader::GameObjectPixel)),
      resources.TryGet(program)->PixelShader);

  static_assert(
      !std::is_same_v<ResourceHandle<Mesh>, ResourceHandle<Material>>);
}

TEST(ResourceManagerTest, AddsDerivedResourceUsingItsOwnDescription) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  ResourceManager resources;

  /** 模拟扩展阶段类型，确认派生类仍会归一到 Shader 池。 */
  struct TestShader final : Shader {
    TestShader() {
      Id = "builtin://shader/test/vertex";
      Name = "Test_V";
    }
  };
  const auto handle = resources.Add(std::make_unique<TestShader>());

  ASSERT_TRUE(handle.IsValid());
  EXPECT_EQ(resources.TryGet(handle)->Id, "builtin://shader/test/vertex");
  EXPECT_EQ(
      resources.Resolve(ResourceRef<Shader>("builtin://shader/test/vertex")),
      handle);
}

} // namespace z8
