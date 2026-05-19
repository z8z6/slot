//
// Created by zhou_zhengming on 2026/5/12.
//

#include "Target/DirectX/DX12ConstBuf.h"

#include <DirectXMath.h>

#include "d3dx12.h"
#include "Target/DirectX/DX12Device.h"

z8::DX12ConstBuf::~DX12ConstBuf()
{
  Buffer->Unmap(0, nullptr);
}

void z8::DX12ConstBuf::InitDescriptor()
{
  // 描述符大小
  DptSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  D3D12_DESCRIPTOR_HEAP_DESC CD;
  CD.NumDescriptors = 1;
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
  // 上传堆
  auto Prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  auto D = CD3DX12_RESOURCE_DESC::Buffer(ByteSize);
  // 创建GPU缓冲区
  Ok(Ctx->Device->CreateCommittedResource(
    &Prop, D3D12_HEAP_FLAG_NONE,
    &D, D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&Buffer)));
  // 绑定到CPU
  Ok(Buffer->Map(0, nullptr, reinterpret_cast<void**>(&ConstBufCPU)));

  D3D12_GPU_VIRTUAL_ADDRESS cbAddress = Buffer->GetGPUVirtualAddress();
  // 物体偏移
  int boxCBufIndex = 0;
  cbAddress += boxCBufIndex * ByteSize;
  // 绑定描述符
  D3D12_CONSTANT_BUFFER_VIEW_DESC CD;
  CD.BufferLocation = cbAddress;
  CD.SizeInBytes = ByteSize;

  Ctx->Device->CreateConstantBufferView(&CD, Dpt);
}
