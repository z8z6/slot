//
// Created by zhou_zhengming on 2026/5/9.
//

#include "DirectX/DX12Context.h"
#include <d3d12.h>

using namespace z8;

DX12Context::DX12Context() {
  CreateDXGIFactory1(IID_PPV_ARGS(&Factory));
  D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device));
}

DX12Context::~DX12Context() {}