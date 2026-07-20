//
// Created by zhou_zhengming on 2026/5/12.
//

#include "Target/DirectX/DX12ConstBuf.h"

#include <DirectXMath.h>

#include "Core/Application.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"
#include "d3dx12.h"

z8::DX12ConstBuf::~DX12ConstBuf()
{
  Buffer->Unmap(0, nullptr);
}

void z8::DX12ConstBuf::InitDescriptor()
{
  // 单个描述符大小
  DptSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  // 描述符个数： 每个物体的常量 + 全局常量
  DptCount = Render->App->Objects.size() + 1;

  D3D12_DESCRIPTOR_HEAP_DESC CD;
  CD.NumDescriptors = DptCount;
  CD.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  CD.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  CD.NodeMask = 0;
  // 创建描述符堆
  Ok(Ctx->Device->CreateDescriptorHeap(&CD, IID_PPV_ARGS(DptHeap.GetAddressOf())));
  // CPU 侧起始描述符
  Dpt = DptHeap->GetCPUDescriptorHandleForHeapStart();
  // 全局常量位于最后一个索引
  DX12GlobalConst::Index = DptCount - 1;
}

void z8::DX12ConstBuf::InitBuffer()
{
  // 计算单个缓冲区大小
  SingleBufSize = sizeof(DirectX::XMFLOAT4X4);
  StepSize = AlignedSize(SingleBufSize);
  // 计算总缓冲区的大小
  unsigned TotalSize = StepSize * (DptCount - 1) + DX12GlobalConst::AlignedSize();

  // 常量缓冲区需要更新，使用上传堆
  auto Prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  auto D = CD3DX12_RESOURCE_DESC::Buffer(TotalSize);
  // 创建 GPU 侧缓冲区
  Ok(Ctx->Device->CreateCommittedResource(
    &Prop, D3D12_HEAP_FLAG_NONE,
    &D, D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&Buffer)));

  // 将 GPU 侧缓冲区绑定到 CPU 侧指针
  Ok(Buffer->Map(0, nullptr, reinterpret_cast<void**>(&ConstBufCPU)));

  D3D12_GPU_VIRTUAL_ADDRESS start = Buffer->GetGPUVirtualAddress();
  for (int i = 0; i < DptCount; ++i) {
    D3D12_GPU_VIRTUAL_ADDRESS cbAddress = start + i * StepSize;
    // 创建每个常量缓冲区的视图描述
    D3D12_CONSTANT_BUFFER_VIEW_DESC CD;
    CD.BufferLocation = cbAddress;
    CD.SizeInBytes = StepSize;

    // 最后一个缓冲区是全局常量
    if (i == DX12GlobalConst::Index) {
      CD.SizeInBytes = DX12GlobalConst::AlignedSize();
    }

    auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(DptHeap->GetCPUDescriptorHandleForHeapStart());
    Handle.Offset(i, DptSize);

    // 创建每个常量缓冲区的视图
    Ctx->Device->CreateConstantBufferView(&CD, Handle);
  }
}

D3D12_GPU_DESCRIPTOR_HANDLE z8::DX12ConstBuf::GetGPUDescriptor(int index) const {
  auto Handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(DptHeap->GetGPUDescriptorHandleForHeapStart());
  Handle.Offset(index, DptSize);
  return Handle;
}

char *z8::DX12ConstBuf::GetCPUOffset(unsigned index) const {
  return &ConstBufCPU[index * StepSize];
}

unsigned z8::DX12ConstBuf::AlignedSize(unsigned size) {
  return (size + 255) & ~255;
}
