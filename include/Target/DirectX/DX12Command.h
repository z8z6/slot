//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once

#include "Target/DirectX/DX12Common.h"

namespace z8
{
class DX12Command : public DX12Common
{
public:
  ComPtr<ID3D12CommandQueue> CmdQueue;
  ComPtr<ID3D12CommandAllocator> CmdAllocator;
  ComPtr<ID3D12GraphicsCommandList> CmdList;

  void Init();
  void Reset();
  void Exec();
};
}
