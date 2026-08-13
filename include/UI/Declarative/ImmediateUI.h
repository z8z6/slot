#pragma once

#include <optional>
#include <string>
#include <vector>

#include <DirectXMath.h>
#include "UI/Layout/LayoutTypes.h"

namespace z8::ui {
class BaseNode;
class Layout;

/** 常用布局样式的强类型声明；未填写字段不会覆盖控件已有样式。 */
struct UIStyle {
  std::optional<float> Width;
  std::optional<float> Height;
  std::optional<float> MinWidth;
  std::optional<float> MinHeight;
  std::optional<float> FlexGrow;
  std::optional<float> FlexShrink;
  std::optional<float> Margin;
  std::optional<float> Padding;
  std::optional<DirectX::XMFLOAT4> Color;
  std::optional<DirectX::XMFLOAT4> BorderColor;
  std::optional<float> BorderWidth;
  std::optional<float> CornerRadius;
  std::optional<FlexDirection> Direction;
};

/**
 * ImGui 风格声明入口，但内部保留并复用控件。
 * 同级 key 和调用顺序稳定时不会重新分配布局节点或 UIObject。
 */
class ImmediateUI {
public:
  explicit ImmediateUI(Layout& layout);

  void BeginFrame();
  bool BeginPanel(const std::string& key, const std::string& title,
                  const UIStyle& style = {});
  /** 声明消息输出面板；其内容由 Layout 的运行时消息通道维护。 */
  BaseNode *Terminal(const std::string &key, const std::string &title = "Output Log",
                     const UIStyle &style = {});
  void EndPanel();
  /** 声明内建图标；Source 使用 builtin://icon/* 资源名。 */
  BaseNode *Image(const std::string &key, const std::string &source,
                  const UIStyle &style = {});
  BaseNode* Rect(const std::string& key, const UIStyle& style = {});
  /** 声明一个由渲染后端填充的 3D 场景视口。 */
  BaseNode *Scene(const std::string &key, const UIStyle &style = {});
  bool EndFrame();

  const std::string& LastError() const { return Error; }

private:
  struct Scope { BaseNode* Host; size_t NextChild; };
  Layout* TargetLayout;
  std::vector<Scope> ScopeStack;
  bool Changed = false;
  std::string Error;

  BaseNode* Acquire(const std::string& type, const std::string& key);
  /** 恢复控件构造时的主题样式，防止复用节点携带上一帧的声明值。 */
  static void ResetStyle(BaseNode &node, bool keepsInteractiveGeometry);
  static void ApplyStyle(BaseNode& node, const UIStyle& style);
  void CloseScope();
};
} // namespace z8::ui
