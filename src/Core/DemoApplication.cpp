//
// Created by zhou_zhengming on 2026/5/19.
//

#include "Core/DemoApplication.h"

#include "Light/ParallelLight.h"
#include "Object/Camera/BaseCamera.h"
#include "Object/Camera/FirstPersonCamera.h"
#include "Object/BuiltinObject.h"
#include "Target/Render.h"
#include "UI/Layout/TextNode.h"
#include "UI/Layout/TreeViewNode.h"

#include <stdexcept>

using namespace z8;
using namespace z8::ui;

void DemoApplication::BindEditorLayout() {
  auto* tree = dynamic_cast<TreeViewNode*>(Layout.Find("scene-hierarchy"));
  if (!tree)
    return;

  ItemObjects.clear();
  ObjectItems.clear();
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

  auto lights = std::make_unique<TreeViewItemNode>();
  lights->Key = "__scene_lights";
  // 层级面板只展示场景光源集合的摘要，避免把不可编辑的灯光伪装成单个主光源。
  lights->SetText("Directional Lights (" +
                  std::to_string(ActiveScene.Lights.size()) + ")");
  lights->SetEnabled(false);
  root->ContentHost()->AddChild(std::move(lights));

  auto actors = std::make_unique<TreeViewItemNode>();
  actors->Key = "__scene_actors";
  actors->SetText("Actors");
  actors->Expanded = true;
  size_t index = 0;
  for (auto* object : ActiveScene.GOs.get()) {
    auto item = std::make_unique<TreeViewItemNode>();
    auto* observer = item.get();
    if (object->Name.empty())
      object->Name = DescribeObject(*object, index);
    item->Key = "__scene_object_" + std::to_string(index);
    item->SetText(object->Name);
    ItemObjects.emplace(observer, object);
    ObjectItems.emplace(object, observer);
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
  // XAML 热重载会整体替换控件树，因此绑定对象也随新 Layout 重建，绝不缓存
  // 上一棵树中的 TextInput 指针。资源修改只发出后端失效通知。
  const std::function<void()> resourcesChanged = [this]() {
    // Render 同时是类型名和成员名；显式经 this 访问可避免
    // C++17 名字查找歧义，并表明失效通知依赖当前 Application 生命周期。
    if (this->Render)
      this->Render->InvalidateSceneResources();
  };
  DetailsBinding = std::make_unique<SceneObjectDetailsBinding>(
      Layout, Resources, resourcesChanged);
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

void DemoApplication::BeforeInit() {
  ActiveScene.Camera.set<FirstPersonCamera>();

  auto* keyLight = ActiveScene.Lights.add<ParallelLight>();
  keyLight->Color = {0.9f, 0.85f, 0.72f};
  keyLight->Direction = {0.57735f, -0.57735f, 0.57735f};
  keyLight->Transform.Position = {0, 2, -5};

  auto* fillLight = ActiveScene.Lights.add<ParallelLight>();
  fillLight->Color = {0.18f, 0.24f, 0.35f};
  fillLight->Direction = {-0.4f, -0.7f, -0.5f};

  for (int i = -2; i < 3; i++) {
    if (i == 0) continue;
    auto* c = ActiveScene.GOs.add<CubeObject>();
    c->Transform.Position.x = 10.0f * i;
    c->Transform.UniformScale(2);
  }
  auto* sphere = ActiveScene.GOs.add<SphereObject>();
  sphere->Transform.UniformScale(0.5);

  if (!EnableXamlHotReload("asset/xml/Main.xaml"))
    throw std::runtime_error("Unable to load asset/xml/Main.xaml.");
}

void DemoApplication::OnFrame() {
  if (DetailsBinding)
    DetailsBinding->Synchronize();
}

void DemoApplication::OnLayoutReloaded() { BindEditorLayout(); }

void DemoApplication::OnSceneSelectionChanged(GameObject* object) {
  if (auto* tree =
          dynamic_cast<TreeViewNode*>(Layout.Find("scene-hierarchy"))) {
    const auto item = ObjectItems.find(object);
    tree->SelectItem(item == ObjectItems.end() ? nullptr : item->second, false);
  }
  const auto item = ObjectItems.find(object);
  if (DetailsBinding)
    DetailsBinding->Bind(object,
                         item == ObjectItems.end() ? nullptr : item->second);
  if (auto* type = dynamic_cast<TextNode*>(Layout.Find("details-type"))) {
    if (!object)
      type->Text = "No object selected";
    else if (dynamic_cast<SphereObject*>(object))
      type->Text = "Type: Sphere";
    else if (dynamic_cast<CubeObject*>(object))
      type->Text = "Type: Cube";
    else
      type->Text = "Type: GameObject";
  }
}
