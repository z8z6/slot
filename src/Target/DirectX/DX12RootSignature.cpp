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
  // 创建两个寄存器槽
  CD3DX12_ROOT_PARAMETER slotRootParameter[3];

  // 寄存器 b0
  CD3DX12_DESCRIPTOR_RANGE c0;
  c0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

  // 寄存器 b1
  CD3DX12_DESCRIPTOR_RANGE c1;
  c1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

  // 寄存器 b2
  CD3DX12_DESCRIPTOR_RANGE c2;
  c2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);

  slotRootParameter[0].InitAsDescriptorTable(1, &c0);
  slotRootParameter[1].InitAsDescriptorTable(1, &c1);
  slotRootParameter[2].InitAsDescriptorTable(1, &c2);

  // A root signature is an array of root parameters.
  CD3DX12_ROOT_SIGNATURE_DESC RD(3, slotRootParameter, 0,
    nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
  ComPtr<ID3DBlob> serializedRootSig = nullptr;
  ComPtr<ID3DBlob> errorBlob = nullptr;
  Ok(D3D12SerializeRootSignature(&RD, D3D_ROOT_SIGNATURE_VERSION_1,
    serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf()));

  if (errorBlob != nullptr)
  {
    ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
  }

  Ok(Ctx->Device->CreateRootSignature(
    0,
    serializedRootSig->GetBufferPointer(),
    serializedRootSig->GetBufferSize(),
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
