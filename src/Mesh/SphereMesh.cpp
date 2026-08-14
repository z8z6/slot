//
// Created by zhou_zhengming on 2026/5/19.
//

#include "Mesh/SphereMesh.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>

using namespace z8;
using namespace DirectX;

namespace {

/**
 * 为跨越经度接缝的三角形复制低 U 顶点。
 * 纹理采样器可用 wrap 处理 U>1；若继续共享 0/1 两侧顶点，光栅化插值会横穿整张
 * 纹理。极点没有唯一经度，因此按三角形邻点的平均 U 单独复制。
 */
void SplitTextureSeam(Mesh& mesh) {
  std::unordered_map<Mesh::IndexTy, Mesh::IndexTy> wrappedVertices;
  for (size_t triangle = 0; triangle < mesh.I.size(); triangle += 3) {
    float adjusted[3] = {mesh.V[mesh.I[triangle]].TexCoord.x,
                         mesh.V[mesh.I[triangle + 1]].TexCoord.x,
                         mesh.V[mesh.I[triangle + 2]].TexCoord.x};
    const auto [minimum, maximum] = std::minmax_element(adjusted, adjusted + 3);
    if (*maximum - *minimum <= 0.5f) continue;
    for (auto& u : adjusted)
      if (u < 0.5f) u += 1.0f;

    for (size_t corner = 0; corner < 3; ++corner) {
      const auto original = mesh.I[triangle + corner];
      const auto& position = mesh.V[original].Pos;
      const bool isPole = position.x * position.x + position.z * position.z <
                          1.0e-10f;
      if (isPole)
        adjusted[corner] =
            (adjusted[(corner + 1) % 3] + adjusted[(corner + 2) % 3]) * 0.5f;
      if (std::abs(adjusted[corner] - mesh.V[original].TexCoord.x) < 1.0e-6f)
        continue;

      if (!isPole) {
        if (const auto found = wrappedVertices.find(original);
            found != wrappedVertices.end()) {
          mesh.I[triangle + corner] = found->second;
          continue;
        }
      }
      const auto duplicate = static_cast<Mesh::IndexTy>(mesh.V.size());
      mesh.V.push_back(mesh.V[original]);
      mesh.V.back().TexCoord.x = adjusted[corner];
      mesh.I[triangle + corner] = duplicate;
      if (!isPole) wrappedVertices.emplace(original, duplicate);
    }
  }
}

} // namespace

// Approximate a sphere by tessellating an icosahedron.
z8::SphereMesh::SphereMesh(float radius, unsigned numSubdivisions) {

  Name = "Sphere";
  if (!std::isfinite(radius) || radius <= 0.0f) return;

  numSubdivisions = std::min<unsigned>(numSubdivisions, 6u);

  // 初始使用20面体，再逐步细分逼近球面
  const float X = 0.525731f;
  const float Z = 0.850651f;

  XMFLOAT3 pos[12] = {
      XMFLOAT3(-X, 0.0f, Z), XMFLOAT3(X, 0.0f, Z),   XMFLOAT3(-X, 0.0f, -Z),
      XMFLOAT3(X, 0.0f, -Z), XMFLOAT3(0.0f, Z, X),   XMFLOAT3(0.0f, Z, -X),
      XMFLOAT3(0.0f, -Z, X), XMFLOAT3(0.0f, -Z, -X), XMFLOAT3(Z, X, 0.0f),
      XMFLOAT3(-Z, X, 0.0f), XMFLOAT3(Z, -X, 0.0f),  XMFLOAT3(-Z, -X, 0.0f)};

  unsigned k[60] = {1,  4,  0, 4,  9, 0, 4, 5,  9, 8, 5, 4,  1,  8, 4,
                    1,  10, 8, 10, 3, 8, 8, 3,  5, 3, 2, 5,  3,  7, 2,
                    3,  10, 7, 10, 6, 7, 6, 11, 7, 6, 0, 11, 6,  1, 0,
                    10, 1,  6, 11, 0, 9, 2, 11, 9, 5, 2, 9,  11, 2, 7};

  V.resize(12);
  I.assign(&k[0], &k[60]);

  for (unsigned i = 0; i < 12; ++i)
    V[i] = Vertex(pos[i]);

  for (unsigned i = 0; i < numSubdivisions; ++i)
    Subdivide();

  // 投影到球面后，径向单位向量就是解析法线；它比离散三角形平均更准确。
  for (auto & v : V) {
    XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&v.Pos));
    XMVECTOR p = XMVectorScale(n, radius);
    XMStoreFloat3(&v.Pos, p);
    XMStoreFloat3(&v.Normal, n);
    const float nx = XMVectorGetX(n);
    const float ny = std::clamp(XMVectorGetY(n), -1.0f, 1.0f);
    const float nz = XMVectorGetZ(n);
    v.TexCoord = {0.5f + std::atan2(nz, nx) / XM_2PI,
                  std::acos(ny) / XM_PI};
  }
  SplitTextureSeam(*this);
  NormalMode = MeshNormalMode::PreserveAuthored;
}

void SphereMesh::Subdivide() {
  const auto oldIndices = std::move(I);
  I.clear();
  I.reserve(oldIndices.size() * 4);

  // 边以排序后的两个端点唯一标识，使共享边只产生一个中点；六级细分仅有
  // 40962 个顶点，不会像逐面复制顶点那样溢出 uint16_t。
  std::unordered_map<uint32_t, IndexTy> midpointCache;
  const auto midpoint = [&](IndexTy first, IndexTy second) -> IndexTy {
    const auto low = std::min(first, second);
    const auto high = std::max(first, second);
    const uint32_t key = (static_cast<uint32_t>(low) << 16U) | high;
    if (const auto found = midpointCache.find(key);
        found != midpointCache.end())
      return found->second;

    const auto& a = V[first].Pos;
    const auto& b = V[second].Pos;
    const IndexTy index = static_cast<IndexTy>(V.size());
    V.emplace_back(XMFLOAT3{(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f,
                           (a.z + b.z) * 0.5f});
    midpointCache.emplace(key, index);
    return index;
  };

  //       v1
  //       *
  //      / \
  //     /   \
  //  m0*-----*m1
  //   / \   / \
  //  /   \ /   \
  // *-----*-----*
  // v0    m2     v2

  const size_t numTris = oldIndices.size() / 3;
  for (unsigned i = 0; i < numTris; ++i) {
    const auto v0 = oldIndices[i * 3];
    const auto v1 = oldIndices[i * 3 + 1];
    const auto v2 = oldIndices[i * 3 + 2];
    const auto m0 = midpoint(v0, v1);
    const auto m1 = midpoint(v1, v2);
    const auto m2 = midpoint(v0, v2);

    I.insert(I.end(), {v0, m0, m2, m0, v1, m1, m2, m1, v2,
                       m0, m1, m2});
  }
}
