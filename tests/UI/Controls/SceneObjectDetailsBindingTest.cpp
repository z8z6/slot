#include "Core/SceneObjectDetailsBinding.h"

#include "Object/GameObject/CubeObject.h"
#include "Resource/BuiltinResource.h"
#include "Resource/ResourceManager.h"
#include "UI/Declarative/XamlLoader.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/TextInputNode.h"
#include "UI/Layout/TreeViewNode.h"

#include <filesystem>
#include <gtest/gtest.h>

namespace z8 {
namespace {

/** 测试内建资源仍使用项目相对路径；作用域结束后恢复，避免影响同进程用例。 */
class ScopedSourceDirectory final {
  std::filesystem::path Previous;

public:
  ScopedSourceDirectory() : Previous(std::filesystem::current_path()) {
    std::filesystem::current_path(SLOT_SOURCE_DIR);
  }

  ~ScopedSourceDirectory() { std::filesystem::current_path(Previous); }
};

ui::TextInputNode *FindInput(ui::Layout &layout, const char *key) {
  return dynamic_cast<ui::TextInputNode *>(layout.Find(key));
}

} // namespace

TEST(SceneObjectDetailsBindingTest, SynchronizesEditableObjectFieldsBothWays) {
  ScopedSourceDirectory sourceDirectory;
  constexpr auto source = R"(
    <UI>
      <TextInput Id="details-name" />
      <TextInput Id="details-mesh" />
      <TextInput Id="details-material" />
      <TextInput Id="details-position-x" />
      <TextInput Id="details-position-y" />
      <TextInput Id="details-position-z" />
      <TextInput Id="details-rotation-x" />
      <TextInput Id="details-rotation-y" />
      <TextInput Id="details-rotation-z" />
      <TextInput Id="details-scale-x" />
      <TextInput Id="details-scale-y" />
      <TextInput Id="details-scale-z" />
    </UI>)";
  ui::Layout layout;
  const auto loaded = ui::XamlLoader().LoadInto(layout, source);
  ASSERT_TRUE(loaded) << loaded.Error;
  ResourceManager resources;
  CubeObject object;
  object.Name = "Cube 1";
  ui::TreeViewItemNode item;
  int resourceChanges = 0;
  SceneObjectDetailsBinding binding(
      layout, resources, [&resourceChanges] { ++resourceChanges; });

  binding.Bind(&object, &item);
  auto *name = FindInput(layout, "details-name");
  auto *mesh = FindInput(layout, "details-mesh");
  auto *positionX = FindInput(layout, "details-position-x");
  ASSERT_NE(name, nullptr);
  ASSERT_NE(mesh, nullptr);
  ASSERT_NE(positionX, nullptr);
  EXPECT_EQ(name->Text, "Cube 1");
  EXPECT_EQ(mesh->Text, builtin::CubeMesh);

  name->SetText("Renamed Cube", true);
  EXPECT_EQ(object.Name, "Renamed Cube");
  EXPECT_EQ(item.LabelNode->Text, "Renamed Cube");
  positionX->SetText("12.5", true);
  EXPECT_FLOAT_EQ(object.Transform.Position.x, 12.5f);
  EXPECT_FLOAT_EQ(object.Transform.Radius, 12.5f);
  positionX->SetText("12.5px", true);
  EXPECT_FLOAT_EQ(object.Transform.Position.x, 12.5f);

  mesh->SetText(std::string(builtin::SphereMesh), true);
  EXPECT_EQ(object.Renderable.Mesh.GetAssetId(), builtin::SphereMesh);
  EXPECT_EQ(resourceChanges, 1);
  mesh->SetText("missing://mesh", true);
  EXPECT_EQ(object.Renderable.Mesh.GetAssetId(), builtin::SphereMesh);
  binding.Synchronize();
  EXPECT_EQ(mesh->Text, builtin::SphereMesh);

  positionX->SetFocused(true);
  positionX->SetText("editing", false);
  object.Transform.Position.x = 8.0f;
  binding.Synchronize();
  EXPECT_EQ(positionX->Text, "editing");
  positionX->SetFocused(false);
  binding.Synchronize();
  EXPECT_EQ(positionX->Text, "8");

  binding.Bind(nullptr);
  EXPECT_FALSE(name->Enabled);
  EXPECT_TRUE(name->Text.empty());
}

} // namespace z8
