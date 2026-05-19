//
// Created by zhou_zhengming on 2026/5/9.
//

#pragma once

#include "DX12Common.h"

namespace z8 {
// 这个类存放 DX12 的全局类
class DX12Device {
private:
  DX12Device();
  ~DX12Device();

public:
  ComPtr<IDXGIFactory> Factory;
  ComPtr<ID3D12Device> Device;

  DX12Device(const DX12Device&) = delete;
  DX12Device& operator=(const DX12Device&) = delete;
  DX12Device(DX12Device&&) = delete;
  DX12Device& operator=(DX12Device&&) = delete;

  static DX12Device& Instance() {
    static DX12Device instance;
    return instance;
  }
};
}




