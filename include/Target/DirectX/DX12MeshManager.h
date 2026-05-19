//
// Created by zhou_zhengming on 2026/5/18.
//

#pragma once
#include <d3d12.h>
#include <d3dcommon.h>

#include "DX12Common.h"
#include "DX12UploadBuf.h"
#include "UI/Mesh/Mesh.h"

namespace z8
{
class DX12MeshManager : public DX12Common{
public:
  Mesh Mesh;

  ComPtr<ID3DBlob> VBufCPU;
  ComPtr<ID3DBlob> IBufCPU;

  ComPtr<ID3D12Resource> VBufGPU;
  ComPtr<ID3D12Resource> IBufGPU;

  D3D12_VERTEX_BUFFER_VIEW Vv;
  D3D12_INDEX_BUFFER_VIEW Iv;

  DX12UploadBuf VBufUpload;
  DX12UploadBuf IBufUpload;

  DXGI_FORMAT FormatIBuf = DXGI_FORMAT_R16_UINT;

  DX12MeshManager(DX12Render* R);
  void UnifyMesh();
  void Init();
  void Bind() const;
};
}


