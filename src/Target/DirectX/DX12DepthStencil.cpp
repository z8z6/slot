#include "Target/DirectX/DX12DepthStencil.h"

#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"
#include "d3dx12.h"

#include <algorithm>

using namespace z8;

void DX12DepthStencil::ClearBuffer() const {
  Render->Cmd.List->ClearDepthStencilView(
      Dpt, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0,
      nullptr);
}

void DX12DepthStencil::InitBuffer(int width, int height, unsigned sampleCount,
                                  unsigned sampleQuality) {
  ResetBuffer();
  const unsigned resolvedSampleCount = (std::max)(1U, sampleCount);
  D3D12_RESOURCE_DESC description{};
  description.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  description.Width = static_cast<UINT64>((std::max)(1, width));
  description.Height = static_cast<UINT>((std::max)(1, height));
  description.DepthOrArraySize = 1;
  description.MipLevels = 1;
  description.Format = DXGI_FORMAT_R24G8_TYPELESS;
  description.SampleDesc.Count = resolvedSampleCount;
  description.SampleDesc.Quality = sampleQuality;
  description.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  description.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

  D3D12_CLEAR_VALUE clearValue{};
  clearValue.Format = Format;
  clearValue.DepthStencil.Depth = 1.0f;
  clearValue.DepthStencil.Stencil = 0;
  const auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  // 深度纹理创建后只在 DEPTH_WRITE 状态使用，直接指定最终初始状态可避免
  // Floating host 创建资源时占用共享 command list 记录一次性 barrier。
  Ok(Ctx->Device->CreateCommittedResource(
      &heap, D3D12_HEAP_FLAG_NONE, &description,
      D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue,
      IID_PPV_ARGS(Buffer.GetAddressOf())));

  D3D12_DEPTH_STENCIL_VIEW_DESC view{};
  view.Flags = D3D12_DSV_FLAG_NONE;
  view.ViewDimension = resolvedSampleCount > 1
                           ? D3D12_DSV_DIMENSION_TEXTURE2DMS
                           : D3D12_DSV_DIMENSION_TEXTURE2D;
  view.Format = Format;
  if (resolvedSampleCount == 1)
    view.Texture2D.MipSlice = 0;
  Ctx->Device->CreateDepthStencilView(Buffer.Get(), &view, Dpt);
}

void DX12DepthStencil::InitDescriptor() {
  DptHeap.Reset();
  D3D12_DESCRIPTOR_HEAP_DESC description{};
  description.NumDescriptors = 1;
  description.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  Ok(Ctx->Device->CreateDescriptorHeap(
      &description, IID_PPV_ARGS(DptHeap.GetAddressOf())));
  Dpt = DptHeap->GetCPUDescriptorHandleForHeapStart();
}

void DX12DepthStencil::ResetBuffer() { Buffer.Reset(); }
