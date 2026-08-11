//
// Created by zhou_zhengming on 2026/5/18.
//

#pragma once
#include <d3d12.h>

#include "DX12Common.h"
#include "DX12DefaultBuffer.h"
#include "Mesh/Mesh.h"
#include "Resource/ResourceHandle.h"

#include <unordered_map>

namespace z8
{
struct DX12SubMesh {
  // 总索引数
  unsigned IndexCount = 0;
  // 起始索引偏移
  unsigned StartIndexLocation = 0;
  // 起始顶点偏移
  int BaseVertexLocation = 0;
};

class DX12MeshManager : public DX12Common{
public:
  Mesh MergeMesh;

  DX12DefaultBuffer VBuf;
  DX12DefaultBuffer IBuf;

  D3D12_VERTEX_BUFFER_VIEW Vv;
  D3D12_INDEX_BUFFER_VIEW Iv;

  DXGI_FORMAT FormatIBuf = DXGI_FORMAT_R16_UINT;
  std::unordered_map<ResourceHandle<Mesh>, DX12SubMesh,
                     ResourceHandleHash<Mesh>> SubMeshes;

  DX12MeshManager(DX12Render* R);
  void UnifyMesh();
  void Init();
  void Bind() const;
  DX12SubMesh* GetSubMesh(ResourceHandle<Mesh> mesh);
};
}


