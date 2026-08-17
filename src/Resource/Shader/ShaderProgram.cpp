#include "Shader/ShaderProgram.h"

using namespace z8;

ShaderProgram::ShaderProgram() {
  Type = ResourceTy::ShaderProgram;
  Id = builtin::shader::program::ShaderProgramPrefix;
}

ShaderProgram::ShaderProgram(std::string_view id,
                             ResourceHandle<Shader> vertexShader,
                             ResourceHandle<Shader> pixelShader,
                             bool enableDepth, bool enableBlend)
    : VertexShader(vertexShader), PixelShader(pixelShader),
      EnableDepth(enableDepth), EnableBlend(enableBlend) {
  Type = ResourceTy::ShaderProgram;
  Id = id;
}
