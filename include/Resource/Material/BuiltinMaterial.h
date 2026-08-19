#pragma once

#include "BaseMaterial.h"
#include "Resource/BuiltinResource.h"

namespace z8 {
struct MetalMaterial : BaseMaterial {
  MetalMaterial() {
    FresnelR0 = {0.35f, 0.38f, 0.35f};
    Rough = 0.55f;
    Id = builtin::material::MetalMaterial;
  }
};

struct GrassBlockMaterial : BaseMaterial {
  GrassBlockMaterial() {
    Albedo = {1.0f, 1.0f, 1.0f, 1.0f};
    FresnelR0 = {0.02f, 0.02f, 0.02f};
    Rough = 0.85f;
    Texture = ResourceRef<BaseTexture>(builtin::texture::GrassBlockTexture);
    Id = builtin::material::GrassBlockMaterial;
  }
};

struct UIMaterial : BaseMaterial {
  UIMaterial() {
    Id = builtin::material::UIMaterial;
    Program = ResourceRef<BaseShader>(builtin::shader::program::UIObjectProgram);
  }
};

} // namespace z8
