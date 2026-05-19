//
// Created by zhou_zhengming on 2026/5/18.
//

#pragma once
#include <d3d12.h>
#include <d3dcommon.h>

#include "DX12Common.h"
#include "DX12UploadBuf.h"
#include "UI/Mesh/Mesh.h"

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

  ComPtr<ID3DBlob> VBufCPU;
  ComPtr<ID3DBlob> IBufCPU;

  ComPtr<ID3D12Resource> VBufGPU;
  ComPtr<ID3D12Resource> IBufGPU;

  D3D12_VERTEX_BUFFER_VIEW Vv;
  D3D12_INDEX_BUFFER_VIEW Iv;

  DX12UploadBuf VBufUpload;
  DX12UploadBuf IBufUpload;

  DXGI_FORMAT FormatIBuf = DXGI_FORMAT_R16_UINT;
  std::unordered_map<Mesh*, DX12SubMesh> SubMeshes;

  DX12MeshManager(DX12Render* R);
  void UnifyMesh();
  void Init();
  void Bind() const;
  DX12SubMesh* GetSubMesh(Mesh* Mesh);
};
}


