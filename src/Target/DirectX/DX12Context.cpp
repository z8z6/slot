//
// Created by zhou_zhengming on 2026/5/9.
//

#include "Target/DirectX/DX12Context.h"
#include <d3d12.h>
#include <dxgi1_4.h>

using namespace z8;

DX12Context::DX12Context() {
  // Enable the D3D12 debug layer.
#if defined(DEBUG) || defined(_DEBUG)
  ComPtr<ID3D12Debug> Debug;
  Ok(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)));
  Debug->EnableDebugLayer();
#endif
  CreateDXGIFactory1(IID_PPV_ARGS(&Factory));
  D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device));
}

DX12Context::~DX12Context() {}