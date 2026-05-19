//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once
#include "DX12Common.h"
#include "d3d12.h"
#include <vector>

namespace z8
{
class DX12PipelineState : public DX12Common{
public:
  ComPtr<ID3D12PipelineState> Pipe;
  std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;

  DX12PipelineState(DX12Render* R);

  void Init();
  ID3D12PipelineState* operator->() const;
};
}

