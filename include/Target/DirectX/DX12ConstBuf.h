//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once

#include "DX12Common.h"
#include "d3d12.h"

namespace z8
{
/**
 * @brief 常量缓冲区
 */
class DX12ConstBuf: public DX12Common
{
  unsigned SingleBufSize = 0;
  unsigned StepSize = 0;
public:
  ComPtr<ID3D12Resource> Buffer;
  char* ConstBufCPU;
  ComPtr<ID3D12DescriptorHeap> DptHeap;
  unsigned DptSize = 0;
  D3D12_CPU_DESCRIPTOR_HANDLE Dpt;
  unsigned DptCount = 0;

  DX12ConstBuf(DX12Render* R) : DX12Common(R){}
  ~DX12ConstBuf() override;

  void InitDescriptor();
  void InitBuffer();
  D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptor(int index) const;
  // 根据索引获取常量缓冲区的指针
  char* GetCPUOffset(unsigned index) const;
  // 返回 256 字节对齐的缓冲区大小
  static unsigned AlignedSize(unsigned size);
};
}
