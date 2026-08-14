#pragma once

#include <optional>
#include <string>
#include <vector>

#include "UI/Layout/LayoutTypes.h"
#include <DirectXMath.h>

namespace z8::ui {
class BaseNode;
class ButtonNode;
class Layout;
class MenuItemNode;
class MenuNode;
class SliderNode;
class TextInputNode;
class ToggleNode;
class ToolBarNode;

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
  explicit ImmediateUI(Layout &layout);

  void BeginFrame();
  /** 声明可递归嵌套的 Menu 目录；其子项必须在 EndMenu 前稳定重放。 */
  bool BeginMenu(const std::string &key, const std::string &text,
                 const UIStyle &style = {});
  /** 声明按钮；返回本帧之前是否完成过一次激活。 */
  bool Button(const std::string &key, const std::string &text,
              const UIStyle &style = {});
  bool BeginPanel(const std::string &key, const std::string &title,
                  const UIStyle &style = {});
  /** 声明固定顶部 ToolBar，并开始其 Menu 集合作用域。 */
  bool BeginToolBar(const std::string &key, const UIStyle &style = {});
  /** 声明消息输出面板；其内容由 Layout 的运行时消息通道维护。 */
  BaseNode *Terminal(const std::string &key,
                     const std::string &title = "Output Log",
                     const UIStyle &style = {});
  void EndMenu();
  void EndPanel();
  void EndToolBar();
  /** 声明内建图标；Source 使用 builtin://icon/* 资源名。 */
  BaseNode *Image(const std::string &key, const std::string &source,
                  const UIStyle &style = {});
  /** 声明 Menu 叶子命令；返回本帧之前是否完成过激活。 */
  bool MenuItem(const std::string &key, const std::string &text,
                const UIStyle &style = {});
  BaseNode *Rect(const std::string &key, const UIStyle &style = {});
  /** 声明 Slider，并把交互后的值写回调用方状态。 */
  bool Slider(const std::string &key, float &value, float minimum = 0.0f,
              float maximum = 1.0f, const UIStyle &style = {});
  /** 声明一个由渲染后端填充的 3D 场景视口。 */
  BaseNode *Scene(const std::string &key, const UIStyle &style = {});
  /** 声明单行文字输入，并把已编辑 UTF-8 文本写回调用方状态。 */
  bool TextInput(const std::string &key, std::string &value,
                 const std::string &placeholder = "",
                 const UIStyle &style = {});
  /** 声明布尔开关，并把交互后的状态写回调用方状态。 */
  bool Toggle(const std::string &key, const std::string &text, bool &value,
              const UIStyle &style = {});
  bool EndFrame();

  const std::string &LastError() const { return Error; }

private:
  struct Scope {
    BaseNode *Host;
    size_t NextChild;
  };
  Layout *TargetLayout;
  std::vector<Scope> ScopeStack;
  bool Changed = false;
  std::string Error;

  BaseNode *Acquire(const std::string &type, const std::string &key);
  /** 恢复控件构造时的主题样式，防止复用节点携带上一帧的声明值。 */
  static void ResetStyle(BaseNode &node, bool keepsInteractiveGeometry);
  static void ApplyStyle(BaseNode &node, const UIStyle &style);
  void CloseScope();
};
} // namespace z8::ui
