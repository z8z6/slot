#pragma once

#include "UI/Layout/DrawNode.h"

namespace z8::ui {

/** 当前无纹理 SRV 时可由 UI Shader 直接绘制的 Lucide 单色图标。 */
enum class ImageKind {
  Close = 1,
  Plus = 2,
  ChevronDown = 3,
  Cube = 4,
  Terminal = 5,
  Settings = 6
};

/**
 * UI 图标节点。
 *
 * Source 是资源边界，asset://texture/icons/lucide/* 指向随项目分发的 SVG。
 * 节点把受支持的 Lucide 轮廓编译进 UI Shader，避免每个小图标建立纹理上传
 * 与描述符生命周期；保留旧 builtin URI 仅用于声明兼容。
 */
class ImageNode final : public DrawNode {
public:
  ImageKind Kind = ImageKind::Cube;
  std::string Source = "asset://texture/icons/lucide/box.svg";

  ImageNode();
  bool SetProperty(const std::string &name, const std::string &value) override;
  const char *TypeName() const override { return "Image"; }
};

} // namespace z8::ui
