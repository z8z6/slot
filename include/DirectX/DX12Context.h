//
// Created by zhou_zhengming on 2026/5/9.
//

#pragma once

#include "DX12Common.h"

namespace z8 {
// 这个类存放 DX12 的全局类
class DX12Context {
private:
  DX12Context();
  ~DX12Context();

public:
  ComPtr<IDXGIFactory> Factory;
  ComPtr<ID3D12Device> Device;

  DX12Context(const DX12Context&) = delete;
  DX12Context& operator=(const DX12Context&) = delete;
  DX12Context(DX12Context&&) = delete;
  DX12Context& operator=(DX12Context&&) = delete;

  static DX12Context& Instance() {
    static DX12Context instance;
    return instance;
  }
};
}




