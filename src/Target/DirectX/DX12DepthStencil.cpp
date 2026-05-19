//
// Created by zhou_zhengming on 2026/5/18.
//

#include "Target/DirectX/DX12DepthStencil.h"
#include "d3dx12.h"
#include "Core/Window.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"

using namespace z8;

void z8::DX12DepthStencil::InitDescriptor()
{
  // 单个描述符大小
  DptSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
  D3D12_DESCRIPTOR_HEAP_DESC DD;
  DD.NumDescriptors = 1;
  DD.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  DD.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  DD.NodeMask = 0;
  // 初始化描述符堆
  Ok(Ctx->Device->CreateDescriptorHeap(&DD, IID_PPV_ARGS(DptHeap.GetAddressOf())));
  // 当前描述符
  Dpt = DptHeap->GetCPUDescriptorHandleForHeapStart();
}

void z8::DX12DepthStencil::InitBuffer()
{
  D3D12_RESOURCE_DESC BD;
  BD.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  BD.Alignment = 0;
  BD.Width = Render->GetWindow()->Width;
  BD.Height = Render->GetWindow()->Height;
  BD.DepthOrArraySize = 1;
  BD.MipLevels = 1;
  BD.Format = DXGI_FORMAT_R24G8_TYPELESS;
  BD.SampleDesc.Count = Render->Msaa.GetSampleCount();
  BD.SampleDesc.Quality = Render->Msaa.GetMsaaQuality();
  BD.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  BD.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

  D3D12_CLEAR_VALUE Clv;
  Clv.Format = Format;
  Clv.DepthStencil.Depth = 1.0f;
  Clv.DepthStencil.Stencil = 0;

  auto HP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  // 创建 Dsv 缓冲区
  Ok(Ctx->Device->CreateCommittedResource(
    &HP,
    D3D12_HEAP_FLAG_NONE,
    &BD,
    D3D12_RESOURCE_STATE_COMMON,
    &Clv,
    IID_PPV_ARGS(Buffer.GetAddressOf())));

  // 绑定 Dsv 描述符
  D3D12_DEPTH_STENCIL_VIEW_DESC DD;
  DD.Flags = D3D12_DSV_FLAG_NONE;
  DD.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  DD.Format = Format;
  DD.Texture2D.MipSlice = 0;
  Ctx->Device->CreateDepthStencilView(Buffer.Get(), &DD, Dpt);

  // 状态转换
  auto WriteBarrier = CD3DX12_RESOURCE_BARRIER::Transition(Buffer.Get(),
                                                           D3D12_RESOURCE_STATE_COMMON,
                                                           D3D12_RESOURCE_STATE_DEPTH_WRITE);
  Render->Cmd.CmdList->ResourceBarrier(1, &WriteBarrier);
}

void DX12DepthStencil::ClearBuffer()
{
  Render->Cmd.CmdList->ClearDepthStencilView(Dpt, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void DX12DepthStencil::ResetBuffer()
{
  Buffer.Reset();
}
