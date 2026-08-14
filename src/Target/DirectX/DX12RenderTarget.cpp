#include "Target/DirectX/DX12RenderTarget.h"

#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"
#include "d3dx12.h"

#include <algorithm>

using namespace z8;

void DX12RenderTarget::Bind(
    const D3D12_CPU_DESCRIPTOR_HANDLE *depth) const {
  const auto target = IsMultisampled() ? MsaaRtv : CurrentRtv;
  Render->Cmd.List->OMSetRenderTargets(1, &target, TRUE, depth);
}

void DX12RenderTarget::ClearBuffer() const {
  const auto target = IsMultisampled() ? MsaaRtv : CurrentRtv;
  const float color[] = {ClearColor.x, ClearColor.y, ClearColor.z,
                         ClearColor.w};
  Render->Cmd.List->ClearRenderTargetView(target, color, 0, nullptr);
}

ID3D12Resource *DX12RenderTarget::GetBuffer() const {
  return Buffer[CurrentBufferIndex].Get();
}

void DX12RenderTarget::InitBuffer(
    DX12SwapChain &swapChain, int width, int height, unsigned sampleCount,
    unsigned sampleQuality, const DirectX::XMFLOAT4 &clearColor) {
  ClearColor = clearColor;
  SampleCount = (std::max)(1U, sampleCount);
  SampleQuality = sampleQuality;
  SelectBuffer(swapChain.GetCurrentBufferIndex());

  auto handle = RtvHeap->GetCPUDescriptorHandleForHeapStart();
  for (int index = 0; index < DX12SwapChain::BufferCount; ++index) {
    Ok(swapChain->GetBuffer(index, IID_PPV_ARGS(&Buffer[index])));
    Ctx->Device->CreateRenderTargetView(Buffer[index].Get(), nullptr, handle);
    handle.ptr += RtvSize;
  }
  MsaaRtv = handle;

  if (!IsMultisampled())
    return;
  D3D12_RESOURCE_DESC description{};
  description.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  description.Width = static_cast<UINT64>((std::max)(1, width));
  description.Height = static_cast<UINT>((std::max)(1, height));
  description.DepthOrArraySize = 1;
  description.MipLevels = 1;
  description.Format = Format;
  description.SampleDesc.Count = SampleCount;
  description.SampleDesc.Quality = SampleQuality;
  description.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  description.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  D3D12_CLEAR_VALUE clearValue{};
  clearValue.Format = Format;
  clearValue.Color[0] = ClearColor.x;
  clearValue.Color[1] = ClearColor.y;
  clearValue.Color[2] = ClearColor.z;
  clearValue.Color[3] = ClearColor.w;
  const auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  // MSAA 颜色纹理只属于当前 Surface；交换链仍保持 Flip model 要求的单采样。
  Ok(Ctx->Device->CreateCommittedResource(
      &heap, D3D12_HEAP_FLAG_NONE, &description,
      D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue,
      IID_PPV_ARGS(MsaaBuffer.GetAddressOf())));
  Ctx->Device->CreateRenderTargetView(MsaaBuffer.Get(), nullptr, MsaaRtv);
}

void DX12RenderTarget::InitDescriptor() {
  // Init 可在宿主重新绑定 HWND 时再次进入；先释放旧 heap，避免 GetAddressOf
  // 覆盖仍持有引用的 COM 指针。
  RtvHeap.Reset();
  RtvSize = Ctx->Device->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  D3D12_DESCRIPTOR_HEAP_DESC description{};
  // 两个交换链 RTV 加一个可选 MSAA RTV；统一大小避免 resize 时重建 heap。
  description.NumDescriptors = DX12SwapChain::BufferCount + 1;
  description.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  Ok(Ctx->Device->CreateDescriptorHeap(&description,
                                       IID_PPV_ARGS(RtvHeap.GetAddressOf())));
}

void DX12RenderTarget::ResetBuffer() {
  for (auto &buffer : Buffer)
    buffer.Reset();
  MsaaBuffer.Reset();
}

void DX12RenderTarget::Resolve() const {
  if (!IsMultisampled() || !MsaaBuffer)
    return;
  // D3D11On12 随后从 RENDER_TARGET 接管交换链缓冲并转换到 PRESENT；Resolve
  // 因而把目标恢复到 RENDER_TARGET，而不是提前切换到 PRESENT。
  D3D12_RESOURCE_BARRIER barriers[] = {
      CD3DX12_RESOURCE_BARRIER::Transition(
          MsaaBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
          D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
      CD3DX12_RESOURCE_BARRIER::Transition(
          GetBuffer(), D3D12_RESOURCE_STATE_PRESENT,
          D3D12_RESOURCE_STATE_RESOLVE_DEST)};
  Render->Cmd.List->ResourceBarrier(2, barriers);
  Render->Cmd.List->ResolveSubresource(GetBuffer(), 0, MsaaBuffer.Get(), 0,
                                      Format);
  barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
      MsaaBuffer.Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
      D3D12_RESOURCE_STATE_RENDER_TARGET);
  barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
      GetBuffer(), D3D12_RESOURCE_STATE_RESOLVE_DEST,
      D3D12_RESOURCE_STATE_RENDER_TARGET);
  Render->Cmd.List->ResourceBarrier(2, barriers);
}

void DX12RenderTarget::SelectBuffer(int index) {
  CurrentBufferIndex = index;
  if (!RtvHeap)
    return;
  CurrentRtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(
      RtvHeap->GetCPUDescriptorHandleForHeapStart(), CurrentBufferIndex,
      RtvSize);
}

void DX12RenderTarget::Transition(bool toPresent) const {
  const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      GetBuffer(),
      toPresent ? D3D12_RESOURCE_STATE_RENDER_TARGET
                : D3D12_RESOURCE_STATE_PRESENT,
      toPresent ? D3D12_RESOURCE_STATE_PRESENT
                : D3D12_RESOURCE_STATE_RENDER_TARGET);
  Render->Cmd.List->ResourceBarrier(1, &barrier);
}
