#include "Object/BuiltinObject.h"

#include <DirectXMath.h>
#include <gtest/gtest.h>

namespace z8 {

TEST(SimpleGameObjectTest, KeepsNormalOrthogonalAfterNonUniformScale) {
  SimpleGameObject object;
  object.Transform.Scale = {2.0f, 5.0f, 3.0f};
  object.Transform.Position = {7.0f, -2.0f, 4.0f};
  object.Update(nullptr);

  const auto* constants =
      static_cast<const ObjectTransformConst*>(object.ConstBuf());
  // CPU 内存保存的是 HLSL 列主序矩阵的转置；转回数学矩阵后模拟 Shader 行向量乘法。
  const auto world = DirectX::XMMatrixTranspose(
      DirectX::XMLoadFloat4x4(&constants->World));
  const auto normalMatrix = DirectX::XMMatrixTranspose(
      DirectX::XMLoadFloat4x4(&constants->WorldInvTranspose));
  const auto tangent = DirectX::XMVector3TransformNormal(
      DirectX::XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f), world);
  const auto normal = DirectX::XMVector3TransformNormal(
      DirectX::XMVectorSet(1.0f, -1.0f, 0.0f, 0.0f), normalMatrix);

  EXPECT_NEAR(DirectX::XMVectorGetX(DirectX::XMVector3Dot(tangent, normal)),
              0.0f, 1.0e-5f);
}

} // namespace z8
