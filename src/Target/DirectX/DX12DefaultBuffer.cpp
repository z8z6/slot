//
// Created by zhou_zhengming on 2026/7/26.
//

#include "Target/DirectX/DX12DefaultBuffer.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"
#include "d3dx12.h"

using namespace z8;

DX12DefaultBuffer::DX12DefaultBuffer(DX12Render *R)
: DX12Buffer(R) ,Size(0), UploadBuffer(R), DefaultBuffer(nullptr){}

void DX12DefaultBuffer::Init(unsigned size) {
  Size = size;
  UploadBuffer.Init(size);

  auto Desc = CD3DX12_RESOURCE_DESC::Buffer(size);
  auto Prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  Ok(Ctx->Device->CreateCommittedResource(
    &Prop, D3D12_HEAP_FLAG_NONE,
    &Desc, D3D12_RESOURCE_STATE_COMMON,
    nullptr,
    IID_PPV_ARGS(DefaultBuffer.GetAddressOf())));
}

void DX12DefaultBuffer::Update(const void *src) {
  D3D12_SUBRESOURCE_DATA SD = {};
  SD.pData = src;
  SD.RowPitch = Size;
  SD.SlicePitch = SD.RowPitch;

  auto Barrier = CD3DX12_RESOURCE_BARRIER::Transition(DefaultBuffer.Get(),
    D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
  Render->Cmd.List->ResourceBarrier(1, &Barrier);

  UpdateSubresources<1>(Render->Cmd.List.Get(), DefaultBuffer.Get(),
  UploadBuffer.GPUBuffer.Get(), 0, 0, 1, &SD);

  auto Barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(DefaultBuffer.Get(),
    D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
  Render->Cmd.List->ResourceBarrier(1, &Barrier1);
}