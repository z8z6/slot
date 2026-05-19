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
  ComPtr<ID3D12CommandQueue> Queue;
  ComPtr<ID3D12CommandAllocator> Allocator;
  ComPtr<ID3D12GraphicsCommandList> List;
  ComPtr<ID3D12Fence> Fence;
  unsigned __int64  CurFence = 0;

  DX12Command(DX12Render* R);
  ~DX12Command();

  void Init();
  void Synchronize();
  void Reset();
  void ResetWithPso();
  void CloseAndExecute();
};
}
