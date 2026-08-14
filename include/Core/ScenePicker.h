#pragma once

namespace z8 {

class Camera;
class GameObject;
class ResourceManager;
class Scene;

/** 场景视口的客户区矩形；拾取只接受该矩形内部的指针。 */
struct ScenePickRect {
  float Left = 0.0f;
  float Top = 0.0f;
  float Width = 0.0f;
  float Height = 0.0f;
};

/**
 * @brief 编辑器 CPU 射线拾取器。
 *
 * 仅在点击时遍历当前静态 Mesh 三角形，不进入逐帧渲染热路径；返回最近命中的
 * GameObject 观察指针，所有权始终保留在 Scene。
 */
class ScenePicker final {
public:
  static GameObject* Pick(Scene& scene, const ResourceManager& resources,
                          Camera& camera, const ScenePickRect& viewport,
                          float pointerX, float pointerY);
};

} // namespace z8
