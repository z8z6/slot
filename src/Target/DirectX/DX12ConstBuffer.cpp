//
// Created by zhou_zhengming on 2026/5/12.
//

#include "Target/DirectX/DX12ConstBuffer.h"

#include <DirectXMath.h>

#include "../../../include/Object/GameObject.h"
#include "Core/Application.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"
#include <algorithm>

z8::DX12ConstBuffer::DX12ConstBuffer(DX12RenderBatch *B)
: DX12Common(B->Render), Batch(B), Buffer(B->Render){}

void z8::DX12ConstBuffer::InitBuffer()
{
  BufferCount = static_cast<unsigned>(Batch->ROs.size()) + 1;
  // 同一批次允许不同的对象常量结构，以最大结构作为统一步长，避免相邻槽覆盖。
  SingleBufSize = 1;
  for (const auto& object : Batch->ROs)
    SingleBufSize = (std::max)(SingleBufSize, object.Object->ConstBufSize());
  StepSize = AlignedSize(SingleBufSize);
  // 计算总缓冲区的大小
  unsigned TotalSize =
      StepSize * (BufferCount - 1) + DX12GlobalConst::AlignedSize();

  Buffer.Init(TotalSize);
}

D3D12_GPU_VIRTUAL_ADDRESS
z8::DX12ConstBuffer::GetGPUAddress(unsigned index) const {
  if (!Buffer.GPUBuffer || index >= BufferCount) return 0;
  return Buffer.GPUBuffer->GetGPUVirtualAddress() +
         static_cast<uint64_t>(index) * StepSize;
}

char *z8::DX12ConstBuffer::GetCPUOffset(unsigned index) const {
  return &Buffer.CPUBuffer[index * StepSize];
}

unsigned z8::DX12ConstBuffer::AlignedSize(unsigned size) {
  return (size + 255) & ~255;
}
