//
// Created by zhou_zhengming on 2026/5/9.
//
#pragma once

#include <wrl.h>
#include <cassert>

class IDXGIFactory;
class ID3D12Device;
class ID3D12CommandQueue;
class ID3D12CommandAllocator;
class ID3D12GraphicsCommandList;
class ID3D12PipelineState;

namespace z8 {
class DX12Context;
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class DX12Common
{
protected:
  DX12Context* Ctx;
public:
  DX12Common();
  virtual ~DX12Common();
};
}

#define Ok(expr)              \
  {                           \
    HRESULT hr__ = (expr);    \
    assert(SUCCEEDED(hr__));  \
  }

