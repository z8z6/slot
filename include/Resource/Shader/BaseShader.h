//
// Created by zhou_zhengming on 2026/5/21.
//

#pragma once
#include "Resource/BuiltinResource.h"
#include "Resource/ResourceHandle.h"

#include <string>
#include <string_view>

namespace z8 {
/**
 * @brief Shader 派生类共享的后端无关编译描述。
 *
 * 内建派生类在 BuiltinShader 中固化 builtin ID、源文件、入口和
 * target；该基类只承载后端无关描述，DXIL 所有权仍位于
 * DX12ShaderLibrary。
 */
struct BaseShader : Resource {
  std::string Entry;
  std::wstring FileName;
  std::string Name;
  std::string Target;

  BaseShader() {
    Type = ResourceTy::Shader;
    Id = builtin::shader::ShaderPrefix;
  }

  /**
   * 构造一个不可缺少编译坐标的阶段描述；ID 由 builtin 常量提供，确保注册与引用
   * 共享同一规范字符串，不再依赖生成类隐式写入。
   */
  BaseShader(std::string_view id, std::wstring_view fileName, std::string_view name,
         std::string_view target, std::string_view entry);
};

} // namespace z8
