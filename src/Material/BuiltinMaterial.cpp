#include "Material/BuiltinMaterial.h"

using namespace z8;

MetalMaterial::MetalMaterial() {
  // 金属表面的 F0 明显高于普通电介质；较高粗糙度形成较宽的高光，便于在当前
  // 演示尺度下观察法线与光照变化。
  FresnelR0 = {0.35f, 0.38f, 0.35f};
  Rough = 0.55f;
  Name = "Metal";
  Program = ResourceRef<ShaderProgram>(builtin::GameObjectProgram);
}

UIMaterial::UIMaterial() {
  // UI Program 关闭深度并启用透明混合；把依赖固化在类型中可防止注册顺序代码
  // 意外把屏幕空间元素绑定到三维不透明管线。
  Name = "UI";
  Program = ResourceRef<ShaderProgram>(builtin::UIObjectProgram);
}
