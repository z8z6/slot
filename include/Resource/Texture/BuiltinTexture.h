#pragma once

#include "BaseTexture.h"

namespace z8 {

/**
 * @brief 演示场景使用的草方块 RGBA8 纹理。
 *
 * 该类把规范 ID 和文件来源绑定为一个不可分割的资源描述，避免
 * 注册端创建通用 Texture 后遗漏或改错 ID。
 */
class GrassBlockTexture final : public BaseTexture {
public:
  GrassBlockTexture();
};

} // namespace z8
