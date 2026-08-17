#include "Shader/Shader.h"

using namespace z8;

Shader::Shader() {
  Type = ResourceTy::Shader;
  Id = builtin::shader::ShaderPrefix;
}

Shader::Shader(std::string_view id, std::wstring_view fileName,
               std::string_view name, std::string_view target,
               std::string_view entry)
    : Entry(entry), FileName(fileName), Name(name), Target(target) {
  Type = ResourceTy::Shader;
  Id = id;
}
