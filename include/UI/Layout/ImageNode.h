#pragma once

#include "UI/Layout/DrawNode.h"

#include <string_view>

namespace z8::ui {

/** 业务层使用的轻量图标语义；只收录当前编辑器实际使用的图标。 */
enum class UIIcon {
  Close = 1,
  Plus = 2,
  ChevronDown = 3,
  Cube = 4,
  Terminal = 5,
  Settings = 6
};

/** 图标语义到项目资源的稳定映射；实际尺寸和 Tint 仍由 Theme 决定。 */
struct UIIconInfo {
  UIIcon Icon;
  std::string_view Source;
};

/** 查询图标的规范资源 URI；返回值引用进程期只读注册表。 */
const UIIconInfo &GetUIIconInfo(UIIcon icon);

/**
 * UI 图标节点。
 *
 * Source 是资源边界，asset://texture/icons/lucide/* 指向随项目分发的 SVG。
 * 节点把受支持的 Lucide 轮廓编译进 UI Shader，避免每个小图标建立纹理上传
 * 与描述符生命周期；保留旧 builtin URI 仅用于声明兼容。
 */
class ImageNode final : public DrawNode {
public:
  UIIcon Icon = UIIcon::Cube;
  std::string Source = "asset://texture/icons/lucide/box.svg";

  ImageNode();
  /** 直接按语义选择图标，业务控件无需知道资源路径。 */
  bool SetIcon(UIIcon icon);
  bool SetProperty(const std::string &name, const std::string &value) override;
  const char *TypeName() const override { return "Image"; }
};

} // namespace z8::ui
