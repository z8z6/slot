#include "Resource/ResourceManager.h"
#include "Resource/BuiltinResource.h"
#include "Shader/BuiltinShader.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <type_traits>

namespace z8 {

/** 验证内建 ID 解析后仍保留具体类型，防止注册器退化为临时基类描述。 */
template <typename ConcreteTy, typename BaseTy>
bool IsBuiltinType(ResourceManager& resources, std::string_view id) {
  const auto handle = resources.Resolve(ResourceRef<BaseTy>(id));
  return dynamic_cast<const ConcreteTy*>(resources.TryGet(handle)) != nullptr;
}

TEST(ResourceManagerTest, ResolvesTypedBuiltinReferences) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  ResourceManager resources;

  const auto mesh =
      resources.Resolve(ResourceRef<BaseMesh>(builtin::mesh::CubeMesh));
  const auto material = resources.Resolve(
      ResourceRef<BaseMaterial>(builtin::material::GrassBlockMaterial));
  const auto program = resources.Resolve(
      ResourceRef<BaseShaderProgram>(builtin::shader::program::GameObjectProgram));

  EXPECT_NE(resources.TryGet(mesh), nullptr);
  ASSERT_NE(resources.TryGet(material), nullptr);
  EXPECT_EQ(resources.Resolve(resources.TryGet(material)->Program), program);
  const auto texture =
      resources.Resolve(resources.TryGet(material)->Texture);
  ASSERT_NE(resources.TryGet(texture), nullptr);
  EXPECT_TRUE(resources.TryGet(texture)->Validate());
  ASSERT_NE(resources.TryGet(program), nullptr);
  EXPECT_TRUE(resources.TryGet(program)->VertexShader.IsValid());
  EXPECT_TRUE(resources.TryGet(program)->PixelShader.IsValid());
  EXPECT_EQ(
      resources.Resolve(ResourceRef<BaseShader>(builtin::shader::GameObjectVertex)),
      resources.TryGet(program)->VertexShader);
  EXPECT_EQ(
      resources.Resolve(ResourceRef<BaseShader>(builtin::shader::GameObjectPixel)),
      resources.TryGet(program)->PixelShader);

  static_assert(
      !std::is_same_v<ResourceHandle<BaseMesh>, ResourceHandle<BaseMaterial>>);
}

TEST(ResourceManagerTest, AddsDerivedResourceUsingItsOwnDescription) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  ResourceManager resources;

  /** 模拟扩展阶段类型，确认派生类仍会归一到 Shader 池。 */
  struct TestShader final : BaseShader {
    TestShader() {
      Id = "builtin://shader/test/vertex";
      Name = "Test_V";
    }
  };
  const auto handle = resources.Add(std::make_unique<TestShader>());

  ASSERT_TRUE(handle.IsValid());
  EXPECT_EQ(resources.TryGet(handle)->Id, "builtin://shader/test/vertex");
  EXPECT_EQ(
      resources.Resolve(ResourceRef<BaseShader>("builtin://shader/test/vertex")),
      handle);
}

TEST(ResourceManagerTest, RegistersConcreteBuiltinShadersAndPrograms) {
  // ResourceManager 在注册 Shader 前会同步构造需要 asset 的内建 Mesh。
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  ResourceManager resources;

  EXPECT_TRUE((IsBuiltinType<GameObjectVertexShader, BaseShader>(
      resources, builtin::shader::GameObjectVertex)));
  EXPECT_TRUE((IsBuiltinType<GameObjectPixelShader, BaseShader>(
      resources, builtin::shader::GameObjectPixel)));
  EXPECT_TRUE((IsBuiltinType<MissingVertexShader, BaseShader>(
      resources, builtin::shader::MissingVertex)));
  EXPECT_TRUE((IsBuiltinType<MissingPixelShader, BaseShader>(
      resources, builtin::shader::MissingPixel)));
  EXPECT_TRUE((IsBuiltinType<TimeVertexShader, BaseShader>(
      resources, builtin::shader::TimeVertex)));
  EXPECT_TRUE((IsBuiltinType<TimePixelShader, BaseShader>(
      resources, builtin::shader::TimePixel)));
  EXPECT_TRUE((IsBuiltinType<UIObjectVertexShader, BaseShader>(
      resources, builtin::shader::UIObjectVertex)));
  EXPECT_TRUE((IsBuiltinType<UIObjectPixelShader, BaseShader>(
      resources, builtin::shader::UIObjectPixel)));

  EXPECT_TRUE((IsBuiltinType<GameObjectShaderProgram, BaseShaderProgram>(
      resources, builtin::shader::program::GameObjectProgram)));
  EXPECT_TRUE((IsBuiltinType<MissingShaderProgram, BaseShaderProgram>(
      resources, builtin::shader::program::MissingProgram)));
  EXPECT_TRUE((IsBuiltinType<TimeShaderProgram, BaseShaderProgram>(
      resources, builtin::shader::program::TimeProgram)));
  EXPECT_TRUE((IsBuiltinType<UIObjectShaderProgram, BaseShaderProgram>(
      resources, builtin::shader::program::UIObjectProgram)));
}

} // namespace z8
