//
// Created by zhou_zhengming on 2026/5/18.
//

#pragma once

#include "DX12Common.h"
#include "d3d12.h"

namespace z8
{
class DX12UploadBuf: public DX12Common
{
public:
  ComPtr<ID3D12Resource> Buffer;

  DX12UploadBuf(DX12Render* R) : DX12Common(R){}

  void InitBuffer(UINT64 Size);
  ComPtr<ID3D12Resource> Upload(const void* Src, UINT64 Size);
};
}
