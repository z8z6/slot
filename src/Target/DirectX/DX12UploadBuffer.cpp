//
// Created by zhou_zhengming on 2026/7/26.
//

#include "Target/DirectX/DX12UploadBuffer.h"
#include "Target/DirectX/DX12Device.h"
#include "d3dx12.h"

using namespace z8;

DX12UploadBuffer::DX12UploadBuffer(DX12Render *R)
: DX12Buffer(R), GPUBuffer(nullptr), CPUBuffer(nullptr) {}

DX12UploadBuffer::~DX12UploadBuffer() {
  GPUBuffer->Unmap(0, nullptr);
}

void DX12UploadBuffer::Init(unsigned size) {
  auto Prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  auto Desc = CD3DX12_RESOURCE_DESC::Buffer(size);
  // 创建 GPU 侧缓冲区
  Ok(Ctx->Device->CreateCommittedResource(
    &Prop, D3D12_HEAP_FLAG_NONE,
    &Desc, D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&GPUBuffer)));

  // 将 GPU 侧缓冲区绑定到 CPU 侧指针
  Ok(GPUBuffer->Map(0, nullptr, reinterpret_cast<void**>(&CPUBuffer)));
}

void DX12UploadBuffer::Update(const void *src) {

}