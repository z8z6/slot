#pragma once

namespace z8::ui {
class BaseNode;

/**
 * Slot 原生 UI 布局求解器。
 *
 * 引擎只写入节点的 LayoutBox，不处理渲染、裁剪或输入。这样的边界允许布局
 * 独立单元测试，也保证 DirectX 后端只消费稳定的像素矩形。
 */
class LayoutEngine final {
public:
  static void Calculate(BaseNode &root, float width, float height);
};

} // namespace z8::ui
