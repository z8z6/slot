//
// Created by zhou_zhengming on 2026/5/18.
//

#include "Target/DirectX/DX12UploadBuf.h"
#include "d3dx12.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"

using namespace z8;

void z8::DX12UploadBuf::InitBuffer(UINT64 Size)
{
  auto D = CD3DX12_RESOURCE_DESC::Buffer(Size);
  auto Prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  Ok(Ctx->Device->CreateCommittedResource(
    &Prop, D3D12_HEAP_FLAG_NONE,
    &D, D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(Buffer.GetAddressOf())));
}

ComPtr<ID3D12Resource> DX12UploadBuf::Upload(const void* Src, UINT64 Size)
{
  InitBuffer(Size);

  ComPtr<ID3D12Resource> defaultBuffer;
  auto D = CD3DX12_RESOURCE_DESC::Buffer(Size);
  auto Prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  Ok(Ctx->Device->CreateCommittedResource(
    &Prop, D3D12_HEAP_FLAG_NONE,
    &D, D3D12_RESOURCE_STATE_COMMON,
    nullptr,
    IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

  D3D12_SUBRESOURCE_DATA subResourceData = {};
  subResourceData.pData = Src;
  subResourceData.RowPitch = Size;
  subResourceData.SlicePitch = subResourceData.RowPitch;

  auto Barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
    D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
  Render->Cmd.List->ResourceBarrier(1, &Barrier);
  UpdateSubresources<1>(Render->Cmd.List.Get(), defaultBuffer.Get(),
    Buffer.Get(), 0, 0, 1, &subResourceData);
  auto Barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
    D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
  Render->Cmd.List->ResourceBarrier(1, &Barrier1);

  return defaultBuffer;
}
