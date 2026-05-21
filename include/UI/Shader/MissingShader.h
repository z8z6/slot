//
// Created by zhou_zhengming on 2026/5/21.
//

#pragma once
#include "Shader.h"

namespace z8 {
struct MissingPixelShader : public PixelShader {
  MissingPixelShader();
};
struct MissingVertexShader : public VertexShader {
  MissingVertexShader();
};
}