//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once
#include "DX12Common.h"
#include <d3dcommon.h>
#include <string>

namespace z8 {
class DX12Shader {
public:
  static ComPtr<ID3DBlob> CompileShader(const std::wstring &filename,
                                        const D3D_SHADER_MACRO *defines,
                                        const std::string &entrypoint,
                                        const std::string &target);

};

}

