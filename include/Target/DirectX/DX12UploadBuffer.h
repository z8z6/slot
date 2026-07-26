//
// Created by zhou_zhengming on 2026/7/26.
//

#pragma once

#include "Target/DirectX/DX12Buffer.h"

namespace z8 {
class DX12UploadBuffer : public DX12Buffer {
public:
  ComPtr<ID3D12Resource> GPUBuffer;
  char* CPUBuffer;

  explicit DX12UploadBuffer(DX12Render *R);
  ~DX12UploadBuffer() override;
  void Init(unsigned size) override;
  void Update(const void *src) override;
};
}

