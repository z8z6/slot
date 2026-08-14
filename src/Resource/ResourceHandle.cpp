//
// Created by zhou_zhengming on 2026/8/15.
//
#include "Resource/ResourceHandle.h"
#include "Resource/BuiltinResource.h"

using namespace z8;

ResourceTy ResourceId::GetType() const {
  const std::string_view id = Id;
  // 必须在前缀后检查 URI 分隔符，避免把 builtin://mesh-cache 误判为 Mesh。
  const auto belongsTo = [id](std::string_view prefix) {
    return id.size() > prefix.size() && id.starts_with(prefix) &&
           id[prefix.size()] == '/';
  };

  if (belongsTo(builtin::MeshPrefix))
    return ResourceTy::Mesh;
  if (belongsTo(builtin::MaterialPrefix))
    return ResourceTy::Material;
  // shader-program 必须先判断，否则它会被较短的 shader 前缀吞掉。
  if (belongsTo(builtin::ShaderProgramPrefix))
    return ResourceTy::ShaderProgram;
  if (belongsTo(builtin::ShaderPrefix))
    return ResourceTy::Shader;
  if (belongsTo(builtin::TexturePrefix))
    return ResourceTy::Texture;
  if (belongsTo(builtin::AudioPrefix))
    return ResourceTy::Audio;
  return ResourceTy::None;
}
