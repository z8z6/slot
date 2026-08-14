#pragma once

#include <functional>
#include <string>
#include <vector>

namespace z8 {
class GameObject;
class ResourceManager;

namespace ui {
class Layout;
class TextInputNode;
class TreeViewItemNode;
}

/**
 * 把 Details 的保留式 TextInput 投影到一个场景对象。
 *
 * 控件不拥有业务数据；输入回调只修改当前对象，逐帧同步则把外部模型变化拉回
 * 未聚焦的输入框。聚焦字段由用户暂时拥有，避免半成品数字被下一帧覆盖。
 */
class SceneObjectDetailsBinding final {
  /** 单个字段只缓存控件观察指针和模型读取器，生命周期受 Layout 约束。 */
  struct FieldBinding {
    ui::TextInputNode *Input = nullptr;
    std::function<std::string()> Read;
  };

  ui::Layout &EditorLayout;
  ResourceManager &Resources;
  std::function<void()> ResourcesChanged;
  std::vector<FieldBinding> Fields;

public:
  SceneObjectDetailsBinding(ui::Layout &layout, ResourceManager &resources,
                            std::function<void()> resourcesChanged = {});
  ~SceneObjectDetailsBinding() = default;

  /** 重新绑定选择；treeItem 只用于让名称修改同步更新 World Outliner。 */
  void Bind(GameObject *object, ui::TreeViewItemNode *treeItem = nullptr);
  /** 从模型刷新所有未聚焦字段，形成双向绑定的 model-to-view 方向。 */
  void Synchronize();
  /** 清除当前 Layout 中旧控件的回调；选择切换时由 Bind 自动调用。 */
  void Unbind();

private:
  void BindField(const char *key, std::function<std::string()> read,
                 std::function<bool(const std::string &)> write);
  void BindFloat(const char *key, float &value,
                 std::function<void()> valueChanged = {});
  void SetInputsEnabled(bool enabled);
};

} // namespace z8
