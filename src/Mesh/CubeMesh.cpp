//
// Created by zhou_zhengming on 2026/5/11.
//

#include "Mesh/CubeMesh.h"

using namespace DirectX;
using namespace z8;

CubeMesh::CubeMesh() {
  const auto addFace = [&](XMFLOAT3 bottomLeft, XMFLOAT3 topLeft,
                           XMFLOAT3 topRight, XMFLOAT3 bottomRight) {
    const auto base = static_cast<IndexTy>(V.size());
    // 每个面保留独立顶点，防止公共角点法线平均后把立方体渲染成圆角。
    V.emplace_back(bottomLeft, XMFLOAT2{0.0f, 1.0f});
    V.emplace_back(topLeft, XMFLOAT2{0.0f, 0.0f});
    V.emplace_back(topRight, XMFLOAT2{1.0f, 0.0f});
    V.emplace_back(bottomRight, XMFLOAT2{1.0f, 1.0f});
    I.insert(I.end(), {base, static_cast<IndexTy>(base + 1),
                       static_cast<IndexTy>(base + 2), base,
                       static_cast<IndexTy>(base + 2),
                       static_cast<IndexTy>(base + 3)});
  };

  // 四点均按从物体外部观察的顺时针顺序传入。
  addFace({-1, -1, -1}, {-1, 1, -1}, {1, 1, -1}, {1, -1, -1});
  addFace({1, -1, 1}, {1, 1, 1}, {-1, 1, 1}, {-1, -1, 1});
  addFace({-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1}, {-1, -1, -1});
  addFace({1, -1, -1}, {1, 1, -1}, {1, 1, 1}, {1, -1, 1});
  addFace({-1, 1, -1}, {-1, 1, 1}, {1, 1, 1}, {1, 1, -1});
  addFace({-1, -1, 1}, {-1, -1, -1}, {1, -1, -1}, {1, -1, 1});
  Name = "Cube";
}
