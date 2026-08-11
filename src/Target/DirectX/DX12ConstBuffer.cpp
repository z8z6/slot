//
// Created by zhou_zhengming on 2026/5/12.
//

#include "Target/DirectX/DX12ConstBuffer.h"

#include <DirectXMath.h>

#include "Core/Application.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"
#include "Object/GameObject/GameObject.h"
#include "d3dx12.h"

#include <algorithm>


z8::DX12ConstBuffer::DX12ConstBuffer(DX12RenderBatch *B)
: DX12Common(B->Render), Batch(B), Buffer(B->Render){}

void z8::DX12ConstBuffer::InitDescriptor()
{
  // 单个描述符大小
  DptSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  // 描述符个数： 每个物体的常量 + 全局常量
  DptCount = Batch->ROs.size() + 1;

  D3D12_DESCRIPTOR_HEAP_DESC CD;
  CD.NumDescriptors = DptCount;
  CD.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  CD.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  CD.NodeMask = 0;
  // 创建描述符堆
  // UI 拓扑变化会重建描述符堆，必须先释放旧 COM 引用。
  Ok(Ctx->Device->CreateDescriptorHeap(
      &CD, IID_PPV_ARGS(DptHeap.ReleaseAndGetAddressOf())));
  // CPU 侧起始描述符
  Dpt = DptHeap->GetCPUDescriptorHandleForHeapStart();
}

void z8::DX12ConstBuffer::InitBuffer()
{
  // 同一批次允许不同的对象常量结构，以最大结构作为统一步长，避免相邻槽覆盖。
  SingleBufSize = 1;
  for (const auto& object : Batch->ROs)
    SingleBufSize = (std::max)(SingleBufSize, object.Object->ConstBufSize());
  StepSize = AlignedSize(SingleBufSize);
  // 计算总缓冲区的大小
  unsigned TotalSize = StepSize * (DptCount - 1) + DX12GlobalConst::AlignedSize();

  Buffer.Init(TotalSize);

  D3D12_GPU_VIRTUAL_ADDRESS start = Buffer.GPUBuffer->GetGPUVirtualAddress();
  for (int i = 0; i < DptCount; ++i) {
    D3D12_GPU_VIRTUAL_ADDRESS cbAddress = start + i * StepSize;
    // 创建每个常量缓冲区的视图描述
    D3D12_CONSTANT_BUFFER_VIEW_DESC CD;
    CD.BufferLocation = cbAddress;
    CD.SizeInBytes = StepSize;

    // 最后一个缓冲区是全局常量
    if (i == GetGlobalConstIndex()) {
      CD.SizeInBytes = DX12GlobalConst::AlignedSize();
    }

    auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(DptHeap->GetCPUDescriptorHandleForHeapStart());
    Handle.Offset(i, DptSize);

    // 创建每个常量缓冲区的视图
    Ctx->Device->CreateConstantBufferView(&CD, Handle);
  }
}

D3D12_GPU_DESCRIPTOR_HANDLE z8::DX12ConstBuffer::GetGPUDescriptor(int index) const {
  auto Handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(DptHeap->GetGPUDescriptorHandleForHeapStart());
  Handle.Offset(index, DptSize);
  return Handle;
}

char *z8::DX12ConstBuffer::GetCPUOffset(unsigned index) const {
  return &Buffer.CPUBuffer[index * StepSize];
}

unsigned z8::DX12ConstBuffer::AlignedSize(unsigned size) {
  return (size + 255) & ~255;
}
