#include "Resource/ResourceManager.h"
#include "Resource/BuiltinResource.h"
#include "Shader/BuiltinShader.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <type_traits>

namespace z8 {

/** 轻量测试资源用于隔离验证 ResourcePool，不引入具体渲染资源约束。 */
struct PoolResource {};

TEST(ResourceRefTest, DistinguishesAssetIdAndIndexedStates) {
  const ResourceRef<PoolResource> pending("test://resource/pending");
  EXPECT_TRUE(pending.IsValid());
  EXPECT_FALSE(pending.IsResolved());
  EXPECT_EQ(pending.GetId(), "test://resource/pending");

  const ResourceRef<PoolResource> resolved(7U);
  EXPECT_TRUE(resolved.IsValid());
  EXPECT_TRUE(resolved.IsResolved());
  EXPECT_EQ(resolved.Index, 7U);
  EXPECT_TRUE(resolved.GetId().empty());
  EXPECT_NE(pending, resolved);
}

TEST(ResourcePoolTest, ReturnsStableIndexedReferences) {
  ResourcePool<PoolResource> pool;
  const auto first = pool.Add("test://resource/first",
                              std::make_unique<PoolResource>());

  ASSERT_TRUE(first.IsValid());
  EXPECT_EQ(first.Index, 0U);
  EXPECT_EQ(pool.Find("test://resource/first"), first);
  EXPECT_NE(pool.TryGet(first), nullptr);
  EXPECT_EQ(pool.TryGet(ResourceRef<PoolResource>("test://resource/first")),
            nullptr);
  EXPECT_FALSE(pool.Add("test://resource/first",
                        std::make_unique<PoolResource>()).IsValid());
}

/** 验证内建 ID 解析后仍保留具体类型，防止注册器退化为临时基类描述。 */
template <typename ConcreteTy, typename BaseTy>
bool IsBuiltinType(ResourceManager &resources, std::string_view id) {
  const auto reference = resources.Resolve(ResourceRef<BaseTy>(id));
  return dynamic_cast<const ConcreteTy *>(resources.TryGet(reference)) != nullptr;
}

TEST(ResourceManagerTest, ResolvesTypedBuiltinReferences) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  ResourceManager resources;

  const auto mesh =
      resources.Resolve(ResourceRef<BaseMesh>(builtin::mesh::CubeMesh));
  const auto material = resources.Resolve(
      ResourceRef<BaseMaterial>(builtin::material::GrassBlockMaterial));
  const auto program = resources.Resolve(
      ResourceRef<BaseShader>(builtin::shader::program::GameObjectProgram));

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
      resources.Resolve(ResourceRef<BaseShaderComponent>(builtin::shader::GameObjectVertex)),
      resources.TryGet(program)->VertexShader);
  EXPECT_EQ(
      resources.Resolve(ResourceRef<BaseShaderComponent>(builtin::shader::GameObjectPixel)),
      resources.TryGet(program)->PixelShader);

  static_assert(!std::is_same_v<ResourceRef<BaseMesh>,
                                ResourceRef<BaseMaterial>>);
}

TEST(ResourceManagerTest, AddsDerivedResourceUsingItsOwnDescription) {
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  ResourceManager resources;

  /** 模拟扩展阶段类型，确认派生类仍会归一到 Shader 池。 */
  struct TestShader final : BaseShaderComponent {
    TestShader() {
      Id = "builtin://shader/test/vertex";
    }
  };
  const auto reference = resources.Add(std::make_unique<TestShader>());

  ASSERT_TRUE(reference.IsValid());
  EXPECT_EQ(resources.TryGet(reference)->Id, "builtin://shader/test/vertex");
  EXPECT_EQ(
      resources.Resolve(ResourceRef<BaseShaderComponent>("builtin://shader/test/vertex")),
      reference);
}

TEST(ResourceManagerTest, RegistersConcreteBuiltinShadersAndPrograms) {
  // ResourceManager 在注册 Shader 前会同步构造需要 asset 的内建 Mesh。
  std::filesystem::current_path(SLOT_SOURCE_DIR);
  ResourceManager resources;

  EXPECT_TRUE((IsBuiltinType<GameObjectVertexShader, BaseShaderComponent>(
      resources, builtin::shader::GameObjectVertex)));
  EXPECT_TRUE((IsBuiltinType<GameObjectPixelShader, BaseShaderComponent>(
      resources, builtin::shader::GameObjectPixel)));
  EXPECT_TRUE((IsBuiltinType<MissingVertexShader, BaseShaderComponent>(
      resources, builtin::shader::MissingVertex)));
  EXPECT_TRUE((IsBuiltinType<MissingPixelShader, BaseShaderComponent>(
      resources, builtin::shader::MissingPixel)));
  EXPECT_TRUE((IsBuiltinType<UIObjectVertexShader, BaseShaderComponent>(
      resources, builtin::shader::UIObjectVertex)));
  EXPECT_TRUE((IsBuiltinType<UIObjectPixelShader, BaseShaderComponent>(
      resources, builtin::shader::UIObjectPixel)));

  EXPECT_TRUE((IsBuiltinType<GameObjectShader, BaseShader>(
      resources, builtin::shader::program::GameObjectProgram)));
  EXPECT_TRUE((IsBuiltinType<MissingShader, BaseShader>(
      resources, builtin::shader::program::MissingProgram)));
  EXPECT_TRUE((IsBuiltinType<UIObjectShader, BaseShader>(
      resources, builtin::shader::program::UIObjectProgram)));
}

} // namespace z8
