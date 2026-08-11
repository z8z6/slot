#pragma once

#include "Material/Material.h"
#include "Mesh/Mesh.h"
#include "Resource/ResourceHandle.h"

namespace z8 {

/**
 * @brief 场景对象可渲染部分的持久化资源绑定。
 *
 * Reference 允许场景在资源尚未驻留时存在；渲染器构建 RenderItem 时统一解析成
 * Handle，之后的帧循环不再进行字符串查询。
 */
struct RenderableComponent {
  ResourceReference<Mesh> Mesh;
  ResourceReference<Material> Material;
};

} // namespace z8
