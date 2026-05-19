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
  // 描述符大小
  DptSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  D3D12_DESCRIPTOR_HEAP_DESC CD;
  // 每个物体 + 通用数据
  DptCount = Render->App->Objects.size() + 1;
  CD.NumDescriptors = DptCount;
  CD.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  CD.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  CD.NodeMask = 0;
  // 描述符堆
  Ok(Ctx->Device->CreateDescriptorHeap(&CD, IID_PPV_ARGS(DptHeap.GetAddressOf())));
  // 描述符
  Dpt = DptHeap->GetCPUDescriptorHandleForHeapStart();
}

void z8::DX12ConstBuf::InitBuffer()
{
  // 缓冲区大小
  unsigned ByteSize = (sizeof(DirectX::XMFLOAT4X4) + 255) & ~255;
  StepSize = ByteSize;
  // 上传堆
  auto Prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  auto D = CD3DX12_RESOURCE_DESC::Buffer(ByteSize * DptCount);
  // 创建GPU缓冲区
  Ok(Ctx->Device->CreateCommittedResource(
    &Prop, D3D12_HEAP_FLAG_NONE,
    &D, D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&Buffer)));
  // 绑定到CPU
  Ok(Buffer->Map(0, nullptr, reinterpret_cast<void**>(&ConstBufCPU)));

  D3D12_GPU_VIRTUAL_ADDRESS start = Buffer->GetGPUVirtualAddress();
  // 物体偏移
  for (int i = 0; i < DptCount; ++i) {
    D3D12_GPU_VIRTUAL_ADDRESS cbAddress = start + i * ByteSize;
    // 绑定描述符
    D3D12_CONSTANT_BUFFER_VIEW_DESC CD;
    CD.BufferLocation = cbAddress;
    CD.SizeInBytes = ByteSize;

    auto Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(DptHeap->GetCPUDescriptorHandleForHeapStart());
    Handle.Offset(i, DptSize);

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
