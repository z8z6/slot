//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once

#include "Material.h"
#include "Resource/BuiltinResource.h"

namespace z8
{
struct MetalMaterial : public Material {
  MetalMaterial(){
    // 金属表面的 F0 明显高于普通电介质；较高粗糙度配合当前演示尺度形成可辨认
    // 的宽高光，同时仍保留 ForestGreen 漫反射用于观察明暗面。
    FresnelR0 = {0.35f, 0.38f, 0.35f};
    Rough = 0.55f;
    Name = "Metal";
    Program = ResourceReference<ShaderProgram>(builtin::GameObjectProgram);
  }
};
}

