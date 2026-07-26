//
// Created by zhou_zhengming on 2026/5/12.
//

#include "Target/DirectX/DX12RootSignature.h"
#include "d3d12.h"
#include "d3dx12.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"

using namespace z8;

z8::DX12RootSignature::DX12RootSignature(DX12Render* R) : DX12Common(R)
{
}

void DX12RootSignature::Init()
{
  // 创建寄存器槽
  CD3DX12_ROOT_PARAMETER slotRootParameter[3];

  // 常量寄存器 b0
  CD3DX12_DESCRIPTOR_RANGE c0;
  c0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

  // 常量寄存器 b2
  CD3DX12_DESCRIPTOR_RANGE c2;
  c2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);

  // 描述符表
  slotRootParameter[0].InitAsDescriptorTable(1, &c0);
  // 根描述符，对应一个 64 位 GPU 地址
  slotRootParameter[1].InitAsConstantBufferView(1);
  slotRootParameter[2].InitAsDescriptorTable(1, &c2);

  CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RD(3, slotRootParameter, 0,
    nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  ComPtr<ID3DBlob> RootSig = nullptr;
  ComPtr<ID3DBlob> err = nullptr;
  Ok(D3D12SerializeVersionedRootSignature(&RD,RootSig.GetAddressOf(), err.GetAddressOf()));
  if (err != nullptr) OutputDebugStringA(static_cast<char *>(err->GetBufferPointer()));

  Ok(Ctx->Device->CreateRootSignature(
    0,
    RootSig->GetBufferPointer(),
    RootSig->GetBufferSize(),
    IID_PPV_ARGS(RootSignature.GetAddressOf())));
}

void DX12RootSignature::Bind() const
{
  Render->Cmd.List->SetGraphicsRootSignature(RootSignature.Get());
}

ID3D12RootSignature* DX12RootSignature::operator->() const
{
  return RootSignature.Get();
}

ID3D12RootSignature* DX12RootSignature::Get() const
{
  return RootSignature.Get();
}
