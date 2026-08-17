//
// Created by zhou_zhengming on 2026/5/22.
//
#include "Mesh/Mesh.h"
#include "Resource/BuiltinResource.h"

#include <cmath>
#include <limits>

using namespace z8;
using namespace DirectX;

Mesh::Mesh() {
  Type = ResourceTy::Mesh;
  Id = builtin::mesh::MeshPrefix;
}

void Mesh::ComputeNormals() {
  // 1. 将所有顶点法线清零
  for (auto& v : V)
  {
    v.Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
  }

  // 2. 遍历每个三角形，计算未归一化的面法线并累加到顶点
  const size_t triangleCount = I.size() / 3;
  for (size_t t = 0; t < triangleCount; ++t)
  {
    IndexTy i0 = I[t * 3 + 0];
    IndexTy i1 = I[t * 3 + 1];
    IndexTy i2 = I[t * 3 + 2];

    // 导入器可能接收不可信文件；这里仍做边界保护，避免法线生成越界访问。
    if (i0 >= V.size() || i1 >= V.size() || i2 >= V.size()) continue;

    const XMFLOAT3& p0 = V[i0].Pos;
    const XMFLOAT3& p1 = V[i1].Pos;
    const XMFLOAT3& p2 = V[i2].Pos;

    // 计算边向量
    XMVECTOR v0 = XMLoadFloat3(&p0);
    XMVECTOR v1 = XMLoadFloat3(&p1);
    XMVECTOR v2 = XMLoadFloat3(&p2);

    XMVECTOR edge1 = XMVectorSubtract(v1, v0);
    XMVECTOR edge2 = XMVectorSubtract(v2, v0);

    // 叉积得到未归一化的面法线（模长等于两倍三角形面积）
    XMVECTOR faceNormal = XMVector3Cross(edge1, edge2);

    // 叉积长度是平行四边形面积；零面积面没有可靠方向，不能污染相邻法线。
    if (XMVectorGetX(XMVector3LengthSq(faceNormal)) <= 1.0e-12f) continue;

    // 累加到三个顶点（面积加权）
    XMVECTOR n0 = XMVectorAdd(XMLoadFloat3(&V[i0].Normal) , faceNormal);
    XMVECTOR n1 = XMVectorAdd(XMLoadFloat3(&V[i1].Normal) , faceNormal);
    XMVECTOR n2 = XMVectorAdd(XMLoadFloat3(&V[i2].Normal) , faceNormal);

    XMStoreFloat3(&V[i0].Normal, n0);
    XMStoreFloat3(&V[i1].Normal, n1);
    XMStoreFloat3(&V[i2].Normal, n2);
  }

  // 3. 归一化所有顶点法线
  for (auto& v : V)
  {
    XMVECTOR n = XMLoadFloat3(&v.Normal);
    // 避免零向量（如孤立顶点）导致的除零
    XMVECTOR lengthSq = XMVector3LengthSq(n);
    if (XMVector3LessOrEqual(lengthSq, g_XMEpsilon))
    {
      // 退化成默认向上
      n = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    }
    else
    {
      n = XMVector3Normalize(n);
    }
    XMStoreFloat3(&v.Normal, n);
  }
}

bool Mesh::Validate(std::string* error) const {
  const auto fail = [&](const char* message) {
    if (error) *error = message;
    return false;
  };

  if (V.empty()) return fail("Mesh has no vertices.");
  if (V.size() > static_cast<size_t>(std::numeric_limits<IndexTy>::max()) + 1)
    return fail("Mesh exceeds the 16-bit vertex limit.");
  if (I.empty() || I.size() % 3 != 0)
    return fail("Mesh indices must contain complete triangles.");

  for (const auto& vertex : V) {
    if (!std::isfinite(vertex.Pos.x) || !std::isfinite(vertex.Pos.y) ||
        !std::isfinite(vertex.Pos.z) || !std::isfinite(vertex.Normal.x) ||
        !std::isfinite(vertex.Normal.y) || !std::isfinite(vertex.Normal.z) ||
        !std::isfinite(vertex.TexCoord.x) ||
        !std::isfinite(vertex.TexCoord.y))
      return fail("Mesh contains a non-finite vertex attribute.");
  }
  for (const auto index : I)
    if (index >= V.size()) return fail("Mesh contains an out-of-range index.");

  if (error) error->clear();
  return true;
}
