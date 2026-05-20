//
// Created by zhou_zhengming on 2026/5/19.
//

#include "UI/Mesh/SphereMesh.h"

#include "UI/Mesh/MeshRegistry.h"
#include "Util/MeshGenerator.h"
#include <DirectXColors.h>

using namespace z8;
using namespace DirectX;

static MeshRegister<SphereMesh> R;

// Approximate a sphere by tessellating an icosahedron.
z8::SphereMesh::SphereMesh(float radius, unsigned numSubdivisions) {

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
    V[i] = {pos[i], XMFLOAT4(Colors::White)};

  for (unsigned i = 0; i < numSubdivisions; ++i)
    Subdivide();

  // Project vertices onto sphere and scale.
  for (auto & v : V) {
    XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&v.Pos));
    XMVECTOR p = XMVectorScale(n, radius);
    XMStoreFloat3(&v.Pos, p);
  }

  Name = "Sphere";
}

void SphereMesh::Subdivide() {
  // Save a copy of the input geometry.
  auto CV = V;
  auto CI = I;

  V.resize(0);
  I.resize(0);

  //       v1
  //       *
  //      / \
  //     /   \
  //  m0*-----*m1
  //   / \   / \
  //  /   \ /   \
  // *-----*-----*
  // v0    m2     v2

  unsigned numTris = (unsigned)CI.size() / 3;
  for (unsigned i = 0; i < numTris; ++i) {
    Vertex v0 = CV[CI[i * 3 + 0]];
    Vertex v1 = CV[CI[i * 3 + 1]];
    Vertex v2 = CV[CI[i * 3 + 2]];

    Vertex m0 = MeshGenerator::MidPoint(v0, v1);
    m0.Color = XMFLOAT4(Colors::White);
    Vertex m1 = MeshGenerator::MidPoint(v1, v2);
    m1.Color = XMFLOAT4(Colors::White);
    Vertex m2 = MeshGenerator::MidPoint(v0, v2);
    m2.Color = XMFLOAT4(Colors::White);

    V.push_back(v0); // 0
    V.push_back(v1); // 1
    V.push_back(v2); // 2
    V.push_back(m0); // 3
    V.push_back(m1); // 4
    V.push_back(m2); // 5

    I.push_back(i*6+0);
    I.push_back(i*6+3);
    I.push_back(i*6+5);

    // 面 2
    I.push_back(i*6+3);
    I.push_back(i*6+1);
    I.push_back(i*6+4);

    // 面 3
    I.push_back(i*6+5);
    I.push_back(i*6+4);
    I.push_back(i*6+2);

    // 面 4
    I.push_back(i*6+3);
    I.push_back(i*6+4);
    I.push_back(i*6+5);
  }
}
