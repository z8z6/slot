//
// Created by zhou_zhengming on 2026/5/18.
//

#include "Target/DirectX/DX12RenderTarget.h"
#include "Target/DirectX/DX12SwapChain.h"
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
  RD.NumDescriptors = RtvBufCount; // 2个描述符
  RD.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  RD.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  RD.NodeMask = 0;
  // 初始化描述符堆
  Ok(Ctx->Device->CreateDescriptorHeap(&RD, IID_PPV_ARGS(DptHeap.GetAddressOf())));
  // 当前描述符
  CurRtvId = 0;
  Dpt = DptHeap->GetCPUDescriptorHandleForHeapStart();
}

void z8::DX12RenderTarget::InitBuffer()
{
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
}

void z8::DX12RenderTarget::Swap()
{

  // 更新描述符
  Dpt = CD3DX12_CPU_DESCRIPTOR_HANDLE(
  DptHeap->GetCPUDescriptorHandleForHeapStart(), CurRtvId, DptSize);
}

void z8::DX12RenderTarget::Bind(bool needDepth)
{
  if (needDepth)
    Render->Cmd.CmdList->OMSetRenderTargets(1, &Dpt,
      true, &Render->DepthStencil.Dpt);
  else
    Render->Cmd.CmdList->OMSetRenderTargets(1, &Dpt,
      true, nullptr);
}

void z8::DX12RenderTarget::ClearBuffer()
{
  Render->Cmd.CmdList->ClearRenderTargetView(Dpt, Color::Black_2, 0, nullptr);
}

void z8::DX12RenderTarget::ResetBuffer()
{
  for (auto& Ptr : Buffer)
    Ptr.Reset();
}

ID3D12Resource* z8::DX12RenderTarget::GetBuffer()
{

  return Buffer[CurRtvId].Get();
}

void z8::DX12RenderTarget::Transition(bool toPresent)
{
  CD3DX12_RESOURCE_BARRIER Barrier;
  if (toPresent)
    Barrier = CD3DX12_RESOURCE_BARRIER::Transition(GetBuffer(),
    D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  else
    Barrier = CD3DX12_RESOURCE_BARRIER::Transition(GetBuffer(),
    D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

  Render->Cmd.CmdList->ResourceBarrier(1, &Barrier);
}
