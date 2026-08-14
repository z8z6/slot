#include "Core/SceneObjectDetailsBinding.h"

#include "Material/Material.h"
#include "Mesh/Mesh.h"
#include "Object/GameObject/GameObject.h"
#include "Resource/ResourceManager.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/TextInputNode.h"
#include "UI/Layout/TreeViewNode.h"

#include <charconv>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <system_error>
#include <utility>

using namespace z8;
using namespace z8::ui;

namespace {

constexpr const char *InputKeys[] = {
    "details-name",       "details-mesh",       "details-material",
    "details-position-x", "details-position-y", "details-position-z",
    "details-rotation-x", "details-rotation-y", "details-rotation-z",
    "details-scale-x",    "details-scale-y",    "details-scale-z"};

std::string FormatFloat(float value) {
  std::ostringstream text;
  text << std::setprecision(6) << value;
  return text.str();
}

bool ParseFloat(const std::string &text, float &value) {
  if (text.empty())
    return false;
  const char *first = text.data();
  const char *last = first + text.size();
  const auto result = std::from_chars(first, last, value);
  return result.ec == std::errc{} && result.ptr == last &&
         std::isfinite(value);
}

} // namespace

SceneObjectDetailsBinding::SceneObjectDetailsBinding(
    Layout &layout, ResourceManager &resources,
    std::function<void()> resourcesChanged)
    : EditorLayout(layout), Resources(resources),
      ResourcesChanged(std::move(resourcesChanged)) {}

void SceneObjectDetailsBinding::Bind(GameObject *object,
                                     TreeViewItemNode *treeItem) {
  Unbind();
  SetInputsEnabled(object != nullptr);
  if (!object)
    return;

  BindField(
      "details-name", [object] { return object->Name; },
      [object, treeItem](const std::string &value) {
        if (value.empty())
          return false;
        object->Name = value;
        if (treeItem)
          treeItem->SetText(value);
        return true;
      });
  BindField(
      "details-mesh",
      [object] { return object->Renderable.Mesh.GetAssetId(); },
      [resources = &Resources, resourcesChanged = ResourcesChanged,
       object](const std::string &value) {
        ResourceReference<Mesh> reference(value);
        if (!resources->Resolve(reference).IsValid())
          return false;
        if (object->Renderable.Mesh.GetAssetId() == value)
          return true;
        object->Renderable.Mesh = std::move(reference);
        if (resourcesChanged)
          resourcesChanged();
        return true;
      });
  BindField(
      "details-material",
      [object] { return object->Renderable.Material.GetAssetId(); },
      [resources = &Resources, resourcesChanged = ResourcesChanged,
       object](const std::string &value) {
        ResourceReference<Material> reference(value);
        if (!resources->Resolve(reference).IsValid())
          return false;
        if (object->Renderable.Material.GetAssetId() == value)
          return true;
        object->Renderable.Material = std::move(reference);
        if (resourcesChanged)
          resourcesChanged();
        return true;
      });

  BindFloat("details-position-x", object->Transform.Position.x,
            [object] { object->Transform.UpdateSpherical(); });
  BindFloat("details-position-y", object->Transform.Position.y,
            [object] { object->Transform.UpdateSpherical(); });
  BindFloat("details-position-z", object->Transform.Position.z,
            [object] { object->Transform.UpdateSpherical(); });
  BindFloat("details-rotation-x", object->Transform.Rotation.x);
  BindFloat("details-rotation-y", object->Transform.Rotation.y);
  BindFloat("details-rotation-z", object->Transform.Rotation.z);
  BindFloat("details-scale-x", object->Transform.Scale.x);
  BindFloat("details-scale-y", object->Transform.Scale.y);
  BindFloat("details-scale-z", object->Transform.Scale.z);
}

void SceneObjectDetailsBinding::BindField(
    const char *key, std::function<std::string()> read,
    std::function<bool(const std::string &)> write) {
  auto *input = dynamic_cast<TextInputNode *>(EditorLayout.Find(key));
  if (!input)
    return;
  input->SetText(read(), false);
  input->TextChanged =
      [write = std::move(write)](const std::string &value) { write(value); };
  Fields.push_back({input, std::move(read)});
}

void SceneObjectDetailsBinding::BindFloat(
    const char *key, float &value, std::function<void()> valueChanged) {
  BindField(
      key, [&value] { return FormatFloat(value); },
      [&value, valueChanged = std::move(valueChanged)](
          const std::string &text) {
        float parsed = 0.0f;
        if (!ParseFloat(text, parsed))
          return false;
        value = parsed;
        if (valueChanged)
          valueChanged();
        return true;
      });
}

void SceneObjectDetailsBinding::SetInputsEnabled(bool enabled) {
  for (const auto *key : InputKeys) {
    if (auto *input = dynamic_cast<TextInputNode *>(EditorLayout.Find(key))) {
      input->SetEnabled(enabled);
      if (!enabled)
        input->SetText({}, false);
    }
  }
}

void SceneObjectDetailsBinding::Synchronize() {
  for (auto &field : Fields) {
    // 聚焦输入可能暂时是 "-"、"1." 等不可解析状态；只有失焦后才用模型
    // 的规范格式覆盖，既允许连续编辑，也能回退无效值。
    if (!field.Input->Focused)
      field.Input->SetText(field.Read(), false);
  }
}

void SceneObjectDetailsBinding::Unbind() {
  for (auto &field : Fields) {
    field.Input->TextChanged = {};
    field.Input->Submitted = {};
  }
  Fields.clear();
}
