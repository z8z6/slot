#pragma once

#include "BaseShader.h"

namespace z8 {

struct GameObjectVertexShader final : VertexShaderComponent {
  GameObjectVertexShader() {
    Id = builtin::shader::GameObjectVertex;
    FileName =  L"asset/shader/GameObject.hlsl";
  }
};

struct GameObjectPixelShader final : PixelShaderComponent {
  GameObjectPixelShader() {
    Id = builtin::shader::GameObjectPixel;
    FileName =  L"asset/shader/GameObject.hlsl";
  }
};

struct GameObjectShader final : BaseShader {
  GameObjectShader(ShaderRef v, ShaderRef p) {
    Id = builtin::shader::program::GameObjectProgram;
    VertexShader = v;
    PixelShader = p;
  }

};

struct UIObjectVertexShader final : VertexShaderComponent {
  UIObjectVertexShader() {
    Id = builtin::shader::UIObjectVertex;
    FileName =  L"asset/shader/UIObject.hlsl";
  }
};

struct UIObjectPixelShader final : PixelShaderComponent {
  UIObjectPixelShader() {
    Id = builtin::shader::UIObjectPixel;
    FileName =  L"asset/shader/UIObject.hlsl";
  }
};

struct UIObjectShader final : BaseShader {
  UIObjectShader(ShaderRef v, ShaderRef p) {
    Id = builtin::shader::program::UIObjectProgram;
    VertexShader = v;
    PixelShader = p;
    EnableDepth = false;
    EnableBlend = true;
  }
};

struct MissingVertexShader final : VertexShaderComponent {
  MissingVertexShader() {
    Id = builtin::shader::MissingVertex;
    FileName =  L"asset/shader/Missing.hlsl";
  }
};

struct MissingPixelShader final : PixelShaderComponent {
  MissingPixelShader() {
    Id = builtin::shader::MissingPixel;
    FileName =  L"asset/shader/Missing.hlsl";
  }
};

struct MissingShader final : BaseShader {
  MissingShader(ShaderRef v, ShaderRef p) {
    Id = builtin::shader::program::MissingProgram;
    VertexShader = v;
    PixelShader = p;
  }
};



} // namespace z8
