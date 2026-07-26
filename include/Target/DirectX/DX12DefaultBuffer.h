//
// Created by zhou_zhengming on 2026/7/26.
//
#pragma once

#include "DX12UploadBuffer.h"

namespace z8 {
class DX12DefaultBuffer : public DX12Buffer {
public:
  int Size;
  DX12UploadBuffer UploadBuffer;
  ComPtr<ID3D12Resource> DefaultBuffer;

  explicit DX12DefaultBuffer(DX12Render *R);
  void Init(unsigned size) override;
  void Update(const void *src) override;
};
}



