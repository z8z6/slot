//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once

#include "DX12Common.h"
#include "DX12UploadBuffer.h"
#include "d3d12.h"

namespace z8
{

/**
 * @brief 常量缓冲区
 */
class DX12ConstBuffer: public DX12Common
{
  unsigned BufferCount = 0;
  unsigned SingleBufSize = 0;
  unsigned StepSize = 0;
public:
  DX12RenderBatch* Batch;
  DX12UploadBuffer Buffer;

  explicit DX12ConstBuffer(DX12RenderBatch* B);

  void InitBuffer();
  /** 根 CBV 直接使用上传缓冲地址，给 SRV 堆保留唯一 Shader-visible 槽位。 */
  D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(unsigned index) const;

  // 根据索引获取常量缓冲区的指针
  char* GetCPUOffset(unsigned index) const;
  // 获取全局常量的索引
  unsigned GetGlobalConstIndex() const { return BufferCount - 1; }

  // 返回 256 字节对齐的缓冲区大小
  static unsigned AlignedSize(unsigned size);
};
}
