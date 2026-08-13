//
// Created by zhou_zhengming on 2026/5/18.
//

#include "Target/DirectX/DX12RenderTarget.h"
#include "Target/DirectX/DX12SwapChain.h"
#include "Core/Window.h"
#include <dxgi1_4.h>
#include "d3dx12.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"
#include "Util/Color.h"


void z8::DX12RenderTarget::InitDescriptor()
{
  // 描述符大小
  DptSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  D3D12_DESCRIPTOR_HEAP_DESC RD;
  // 两个交换链 RTV 供 DirectWrite 互操作，额外一个 RTV 指向 4x 颜色缓冲。
  RD.NumDescriptors = RtvBufCount + 1;
  RD.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  RD.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  RD.NodeMask = 0;
  // 初始化描述符堆
  Ok(Ctx->Device->CreateDescriptorHeap(&RD, IID_PPV_ARGS(DptHeap.GetAddressOf())));
  // 当前描述符
  CurRtvId = 0;
  Dpt = DptHeap->GetCPUDescriptorHandleForHeapStart();
  MsaaDpt = CD3DX12_CPU_DESCRIPTOR_HANDLE(Dpt, RtvBufCount, DptSize);
}

void z8::DX12RenderTarget::InitBuffer()
{
  // 每次重置缓冲区，也要重置描述符
  CurRtvId = 0;
  CD3DX12_CPU_DESCRIPTOR_HANDLE Handle(DptHeap->GetCPUDescriptorHandleForHeapStart());
  for (UINT i = 0; i < RtvBufCount; i++)
  {
    // 初始化缓冲区
    // 后台缓冲区实际由 SwapChain 创建，所以无需 CreateCommittedResource
    Ok(Render->SwapChain->GetBuffer(i, IID_PPV_ARGS(&Buffer[i])));
    // 绑定描述符
    Ctx->Device->CreateRenderTargetView(Buffer[i].Get(), nullptr, Handle);
    Handle.Offset(1, DptSize);
  }

  if (Render->Msaa.EnableMsaa) {
    D3D12_RESOURCE_DESC desc{};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width = Render->GetWindow()->Width;
    desc.Height = Render->GetWindow()->Height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = Format;
    desc.SampleDesc.Count = Render->Msaa.GetSampleCount();
    desc.SampleDesc.Quality = Render->Msaa.GetMsaaQuality();
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = Format;
    clearValue.Color[0] = Color::EditorBackground.x;
    clearValue.Color[1] = Color::EditorBackground.y;
    clearValue.Color[2] = Color::EditorBackground.z;
    clearValue.Color[3] = Color::EditorBackground.w;
    const auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    // 多采样资源不参与 Present，始终在 RT 与 ResolveSource 之间转换。
    Ok(Ctx->Device->CreateCommittedResource(
        &heap, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue,
        IID_PPV_ARGS(MsaaBuffer.GetAddressOf())));
    Ctx->Device->CreateRenderTargetView(MsaaBuffer.Get(), nullptr, MsaaDpt);
  }
}

void z8::DX12RenderTarget::Swap()
{
  // 更新描述符
  Dpt = CD3DX12_CPU_DESCRIPTOR_HANDLE(
  DptHeap->GetCPUDescriptorHandleForHeapStart(), CurRtvId, DptSize);
}

void z8::DX12RenderTarget::Bind(bool needDepth) const {
  const auto target = Render->Msaa.EnableMsaa ? MsaaDpt : Dpt;
  if (needDepth)
    Render->Cmd.List->OMSetRenderTargets(1, &target,
      true, &Render->DepthStencil.Dpt);
  else
    Render->Cmd.List->OMSetRenderTargets(1, &target,
      true, nullptr);
}

void z8::DX12RenderTarget::ClearBuffer() const {
  const auto target = Render->Msaa.EnableMsaa ? MsaaDpt : Dpt;
  Render->Cmd.List->ClearRenderTargetView(target, Color::Clear, 0, nullptr);
}

void z8::DX12RenderTarget::ResetBuffer()
{
  for (auto& Ptr : Buffer)
    Ptr.Reset();
  MsaaBuffer.Reset();
}

void z8::DX12RenderTarget::Resolve() const {
  if (!Render->Msaa.EnableMsaa || !MsaaBuffer)
    return;
  // Resolve 前后显式恢复资源状态：交换链随后由 D3D11On12 从 RT 转为
  // Present，多采样缓冲则保持 RT，供下一帧直接清除和绘制。
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

ID3D12Resource* z8::DX12RenderTarget::GetBuffer() const {
  return Buffer[CurRtvId].Get();
}

void z8::DX12RenderTarget::Transition(bool toPresent) const {
  CD3DX12_RESOURCE_BARRIER Barrier;
  if (toPresent)
    Barrier = CD3DX12_RESOURCE_BARRIER::Transition(GetBuffer(),
    D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  else
    Barrier = CD3DX12_RESOURCE_BARRIER::Transition(GetBuffer(),
    D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

  Render->Cmd.List->ResourceBarrier(1, &Barrier);
}
