#pragma once

#include "Object/GameObject/GameObject.h"

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace z8 {

class Camera;
class Light;

/**
 * @brief 一个可独立装载和销毁的场景，统一拥有相机、灯光和场景对象。
 *
 * Renderer 只观察 GetGameObjects 返回的稳定指针；对象的真实生命周期由 Scene 的
 * unique_ptr 管理。UI Layout 仍是窗口级覆盖层，不随 3D Scene 切换而隐式销毁。
 */
class Scene {
public:
  Scene();
  ~Scene();

  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;

  template <typename ObjectTy, typename... ArgumentTys>
  /** 创建并接管 GameObject，同时维护供 Renderer 使用的稳定观察索引。 */
  ObjectTy& CreateGameObject(ArgumentTys&&... arguments) {
    static_assert(std::is_base_of_v<GameObject, ObjectTy>);
    auto object = std::make_unique<ObjectTy>(
        std::forward<ArgumentTys>(arguments)...);
    auto* result = object.get();
    GameObjects.push_back(result);
    GameObjectOwners.push_back(std::move(object));
    return *result;
  }

  /** 替换活动相机；旧相机在赋值点释放，Renderer 不得跨场景缓存它。 */
  void SetCamera(std::unique_ptr<Camera> camera);
  /** 替换主灯光；当前前向路径只读取这一盏灯。 */
  void SetLight(std::unique_ptr<Light> light);

  Camera* GetCamera() const { return ActiveCamera.get(); }
  Light* GetLight() const { return MainLight.get(); }
  const std::vector<GameObject*>& GetGameObjects() const { return GameObjects; }
  std::vector<GameObject*>& GetGameObjects() { return GameObjects; }

private:
  std::unique_ptr<Camera> ActiveCamera;
  std::unique_ptr<Light> MainLight;
  std::vector<std::unique_ptr<GameObject>> GameObjectOwners;
  std::vector<GameObject*> GameObjects;
};

} // namespace z8
