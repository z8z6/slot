//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once

#include "DX12Common.h"
#include "d3d12.h"

namespace z8
{
class DX12DepthStencil : public DX12Common
{
public:
  ComPtr<ID3D12Resource> Resource;
  ComPtr<ID3D12DescriptorHeap> DptHeap;
  unsigned DptSize = 0;
  D3D12_CPU_DESCRIPTOR_HANDLE Dpt;
  DXGI_FORMAT Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

  void Init();
};

class DX12RenderTarget: public DX12Common
{
public:
  ComPtr<ID3D12Resource> Resource;
  ComPtr<ID3D12DescriptorHeap> DptHeap;
  unsigned DptSize = 0;
  D3D12_CPU_DESCRIPTOR_HANDLE Dpt;
  DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;

  void Init();
};

class DX12ConstBuf: public DX12Common
{
public:
  ComPtr<ID3D12Resource> Resource;
  ComPtr<ID3D12DescriptorHeap> DptHeap;
  unsigned DptSize = 0;
  D3D12_CPU_DESCRIPTOR_HANDLE Dpt;
  DXGI_FORMAT Format;

  void Init();
};

class DX12Uploader: public DX12Common
{
public:
  ComPtr<ID3DBlob> VBufCPU;
  ComPtr<ID3DBlob> IBufCPU;

  ComPtr<ID3D12Resource> VBufGPU;
  ComPtr<ID3D12Resource> IBufGPU;

  D3D12_VERTEX_BUFFER_VIEW Vv;
  D3D12_INDEX_BUFFER_VIEW Iv;

  ComPtr<ID3D12Resource> VBufUpload;
  ComPtr<ID3D12Resource> IBufUpload;
};
}
