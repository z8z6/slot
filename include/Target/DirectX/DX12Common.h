//
// Created by zhou_zhengming on 2026/5/9.
//
#pragma once

#include <wrl.h>
#include <cassert>

class IDXGIFactory;
class IDXGISwapChain;
class IDXGISwapChain3;
class ID3D12Device;
class ID3D12Fence;
class ID3D12Resource;
class ID3D12DescriptorHeap;
class ID3D12CommandQueue;
class ID3D12CommandAllocator;
class ID3D12GraphicsCommandList;
class ID3D12PipelineState;
class ID3D12RootSignature;

namespace z8 {
class DX12Device;
class DX12Render;
class DX12Msaa;
class DX12SwapChain;
class DX12Command;
class DX12RenderTarget;
class DX12DepthStencil;
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class DX12Common
{
protected:
  DX12Device* Ctx;
  DX12Render* Render;
public:
  DX12Common(DX12Render*);
  virtual ~DX12Common() = default;
};
}

#define Ok(expr)              \
  {                           \
    HRESULT hr__ = (expr);    \
    assert(SUCCEEDED(hr__));  \
  }

