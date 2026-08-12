#pragma once

#include "UI/Behavior/DockBehavior.h"
#include "UI/Behavior/DragBehavior.h"
#include "UI/Behavior/ResizeBehavior.h"
#include "UI/Layout/BehaviorNode.h"
#include "UI/Layout/RectNode.h"
#include "UI/Layout/TextNode.h"

namespace z8::ui {

/** SceneNode 的内容命中面；仅该区域允许输入继续进入 3D 场景。 */
class SceneViewportNode final : public BehaviorNode {
public:
  const char *TypeName() const override { return "SceneViewport"; }
  bool RoutesToScene() const override { return true; }
};

/**
 * 编辑器中的 3D 场景视口占位节点。
 *
 * SceneNode 自身不创建 UIObject；标题栏由普通 UI 子节点绘制，后端把场景先画入
 * 离屏纹理，再依据 ViewportNode 的布局框合成到交换链。这样标题拖拽不会穿透相机。
 */
class SceneNode final : public BehaviorNode {
public:
  RectNode *TitleBarNode = nullptr;
  TextNode *TitleNode = nullptr;
  SceneViewportNode *ViewportNode = nullptr;

  SceneNode();

  const char *TypeName() const override { return "Scene"; }
  bool SetProperty(const std::string &name, const std::string &value) override;
  /** 后端只在内容区合成场景，标题栏始终由 UI 通道覆盖绘制。 */
  const BaseNode &Viewport() const { return *ViewportNode; }
  /** 即时声明不得覆盖用户拖动或拉伸后提交的几何。 */
  bool HasInteractiveGeometry() const;

private:
  float TitleHeight = 30.0f;
};

} // namespace z8::ui
