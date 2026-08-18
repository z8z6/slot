#include "Shader/BuiltinShader.h"

#include "Resource/BuiltinResource.h"

using namespace z8;

GameObjectPixelShader::GameObjectPixelShader()
    : BaseShader(builtin::shader::GameObjectPixel,
                 L"asset/shader/GameObject.hlsl", "GameObject_P", "ps_6_0",
                 "PS") {}

GameObjectShaderProgram::GameObjectShaderProgram(
    ResourceHandle<BaseShader> vertexShader,
    ResourceHandle<BaseShader> pixelShader)
    : BaseShaderProgram(builtin::shader::program::GameObjectProgram,
                        vertexShader, pixelShader, true, false) {}

GameObjectVertexShader::GameObjectVertexShader()
    : BaseShader(builtin::shader::GameObjectVertex,
                 L"asset/shader/GameObject.hlsl", "GameObject_V", "vs_6_0",
                 "VS") {}

MissingPixelShader::MissingPixelShader()
    : BaseShader(builtin::shader::MissingPixel, L"asset/shader/Missing.hlsl",
                 "Missing_P", "ps_6_0", "PS") {}

MissingShaderProgram::MissingShaderProgram(
    ResourceHandle<BaseShader> vertexShader,
    ResourceHandle<BaseShader> pixelShader)
    : BaseShaderProgram(builtin::shader::program::MissingProgram, vertexShader,
                        pixelShader, true, false) {}

MissingVertexShader::MissingVertexShader()
    : BaseShader(builtin::shader::MissingVertex, L"asset/shader/Missing.hlsl",
                 "Missing_V", "vs_6_0", "VS") {}

TimePixelShader::TimePixelShader()
    : BaseShader(builtin::shader::TimePixel, L"asset/shader/Time.hlsl",
                 "Time_P", "ps_6_0", "PS") {}

TimeShaderProgram::TimeShaderProgram(ResourceHandle<BaseShader> vertexShader,
                                     ResourceHandle<BaseShader> pixelShader)
    : BaseShaderProgram(builtin::shader::program::TimeProgram, vertexShader,
                        pixelShader, true, false) {}

TimeVertexShader::TimeVertexShader()
    : BaseShader(builtin::shader::TimeVertex, L"asset/shader/Time.hlsl",
                 "Time_V", "vs_6_0", "VS") {}

UIObjectPixelShader::UIObjectPixelShader()
    : BaseShader(builtin::shader::UIObjectPixel,
                 L"asset/shader/UIObject.hlsl", "UIObject_P", "ps_6_0",
                 "PS") {}

UIObjectShaderProgram::UIObjectShaderProgram(
    ResourceHandle<BaseShader> vertexShader,
    ResourceHandle<BaseShader> pixelShader)
    : BaseShaderProgram(builtin::shader::program::UIObjectProgram, vertexShader,
                        pixelShader, false, true) {}

UIObjectVertexShader::UIObjectVertexShader()
    : BaseShader(builtin::shader::UIObjectVertex,
                 L"asset/shader/UIObject.hlsl", "UIObject_V", "vs_6_0",
                 "VS") {}
