#include "Shader/BaseShader.h"

using namespace z8;

BaseShader::BaseShader(std::string_view id, std::wstring_view fileName,
               std::string_view name, std::string_view target,
               std::string_view entry)
    : Entry(entry), FileName(fileName), Name(name), Target(target) {
  Type = ResourceTy::Shader;
  Id = id;
}
