//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once
#include "DX12Common.h"

namespace z8
{
class DX12PipelineState : public DX12Common{
public:
  ComPtr<ID3D12PipelineState> Pipe;
};
}

