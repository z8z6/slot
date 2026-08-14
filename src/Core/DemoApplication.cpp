//
// Created by zhou_zhengming on 2026/5/19.
//

#include "Core/DemoApplication.h"

#include "Light/ParallelLight.h"
#include "Object/Camera/Camera.h"
#include "Object/Camera/FirstPersonCamera.h"
#include "Object/GameObject/CubeObject.h"
#include "Object/GameObject/SphereObject.h"
#include "Target/Render.h"
#include "UI/Layout/TextNode.h"
#include "UI/Layout/TreeViewNode.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace z8;
using namespace z8::ui;

namespace {

std::string FormatVector(const DirectX::XMFLOAT3& value) {
  std::ostringstream text;
  text << std::fixed << std::setprecision(2) << value.x << ", " << value.y
       << ", " << value.z;
  return text.str();
}

} // namespace

void DemoApplication::BindEditorLayout() {
  auto* tree = dynamic_cast<TreeViewNode*>(Layout.Find("scene-hierarchy"));
  if (!tree)
    return;

  ItemObjects.clear();
  ObjectItems.clear();
  ObjectNames.clear();
  tree->ContentNode->Children.clear();
  tree->SelectedItem = nullptr;

  auto root = std::make_unique<TreeViewItemNode>();
  root->Key = "__scene_root";
  root->SetText("Scene");
  root->Expanded = true;

  auto camera = std::make_unique<TreeViewItemNode>();
  camera->Key = "__scene_camera";
  camera->SetText("Camera");
  camera->SetEnabled(false);
  root->ContentHost()->AddChild(std::move(camera));

  auto light = std::make_unique<TreeViewItemNode>();
  light->Key = "__scene_light";
  light->SetText("Directional Light");
  light->SetEnabled(false);
  root->ContentHost()->AddChild(std::move(light));

  auto actors = std::make_unique<TreeViewItemNode>();
  actors->Key = "__scene_actors";
  actors->SetText("Actors");
  actors->Expanded = true;
  size_t index = 0;
  for (auto* object : ActiveScene.GetGameObjects()) {
    auto item = std::make_unique<TreeViewItemNode>();
    auto* observer = item.get();
    const auto name = DescribeObject(*object, index);
    item->Key = "__scene_object_" + std::to_string(index);
    item->SetText(name);
    ItemObjects.emplace(observer, object);
    ObjectItems.emplace(object, observer);
    ObjectNames.emplace(object, name);
    actors->ContentHost()->AddChild(std::move(item));
    ++index;
  }
  root->ContentHost()->AddChild(std::move(actors));
  tree->ContentNode->AddChild(std::move(root));
  tree->SelectionChanged = [this](TreeViewItemNode* item) {
    const auto selected = ItemObjects.find(item);
    if (selected != ItemObjects.end())
      SelectSceneObject(selected->second);
  };

  // 动态场景项在 XAML 事务提交后才加入；重建一次非拥有索引，使布局、文字和
  // DX12 UI 批次在同一拓扑版本上工作。
  Layout.RebuildIndex();
  OnSceneSelectionChanged(SelectedSceneObject);
}

std::string DemoApplication::DescribeObject(GameObject& object,
                                            size_t index) const {
  if (dynamic_cast<SphereObject*>(&object))
    return "Sphere " + std::to_string(index + 1);
  if (dynamic_cast<CubeObject*>(&object))
    return "Cube " + std::to_string(index + 1);
  return "GameObject " + std::to_string(index + 1);
}

void DemoApplication::Init() {
  auto camera = std::make_unique<FirstPersonCamera>();
  ActiveScene.SetCamera(std::move(camera));
  auto light = std::make_unique<ParallelLight>();
  light->Transform.Position = { 0, 2, -5};
  light->Direction = { 0.57735f, -0.57735f, 0.57735f };
  ActiveScene.SetLight(std::move(light));

  PrepareScene();
  Render = Render::CreateRender(this);
  Render->Init();
  Window.Open();
}

void DemoApplication::OnLayoutReloaded() { BindEditorLayout(); }

void DemoApplication::OnSceneSelectionChanged(GameObject* object) {
  if (auto* tree =
          dynamic_cast<TreeViewNode*>(Layout.Find("scene-hierarchy"))) {
    const auto item = ObjectItems.find(object);
    tree->SelectItem(item == ObjectItems.end() ? nullptr : item->second, false);
  }
  UpdateDetails(object);
}

void DemoApplication::PrepareScene() {
  for (int i = -2; i < 3; i++) {
    if (i == 0) continue;
    auto& c = ActiveScene.CreateGameObject<CubeObject>();
    c.Transform.Position.x = 10.0f * i;
    c.Transform.Scale.x *= 2;
    c.Transform.Scale.y *= 5;
    c.Transform.Scale.z *= 2;
  }

  // 硬边立方体的单个平面不会形成局部法线变化；中心球体用于直接观察逐像素
  // Blinn-Phong 高光，并同时覆盖解析球面法线的渲染路径。
  auto& sphere = ActiveScene.CreateGameObject<SphereObject>();
  sphere.Transform.Scale = {0.5f, 0.5f, 0.5f};

  // UI 声明完全位于 asset/xml；首次加载与后续热重载共享同一事务解析路径。
  // 初始文件缺失无法构成可用 Demo，因此用英文异常明确终止初始化。
  if (!EnableXamlHotReload("asset/xml/Main.xaml"))
    throw std::runtime_error("Unable to load asset/xml/Main.xaml.");
}

void DemoApplication::UpdateDetails(GameObject* object) {
  const auto setText = [this](const char* key, std::string value) {
    if (auto* text = dynamic_cast<TextNode*>(Layout.Find(key)))
      text->Text = std::move(value);
  };
  if (!object) {
    setText("details-name", "No object selected");
    setText("details-type", "Type: -");
    setText("details-mesh", "Mesh: -");
    setText("details-position", "Position: -");
    setText("details-rotation", "Rotation: -");
    setText("details-scale", "Scale: -");
    return;
  }

  const auto name = ObjectNames.find(object);
  setText("details-name",
          name == ObjectNames.end() ? "GameObject" : name->second);
  setText("details-type",
          dynamic_cast<SphereObject*>(object) ? "Type: Sphere"
                                              : "Type: Cube");
  setText("details-mesh", "Mesh: " + object->Renderable.Mesh.GetAssetId());
  setText("details-position",
          "Position: " + FormatVector(object->Transform.Position));
  setText("details-rotation",
          "Rotation: " + FormatVector(object->Transform.Rotation));
  setText("details-scale", "Scale: " + FormatVector(object->Transform.Scale));
}
