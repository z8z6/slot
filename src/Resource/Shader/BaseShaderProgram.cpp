#include "Shader/BaseShaderProgram.h"

using namespace z8;

BaseShaderProgram::BaseShaderProgram(std::string_view id,
                             ResourceHandle<BaseShader> vertexShader,
                             ResourceHandle<BaseShader> pixelShader,
                             bool enableDepth, bool enableBlend)
    : VertexShader(vertexShader), PixelShader(pixelShader),
      EnableDepth(enableDepth), EnableBlend(enableBlend) {
  Type = ResourceTy::ShaderProgram;
  Id = id;
}
