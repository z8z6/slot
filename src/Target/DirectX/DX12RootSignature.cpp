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
  CD3DX12_ROOT_PARAMETER slotRootParameter[4];

  // 三组常量直接使用根 CBV，避免与纹理 SRV 竞争唯一的 Shader-visible 堆。
  slotRootParameter[0].InitAsConstantBufferView(0);
  slotRootParameter[1].InitAsConstantBufferView(1);
  slotRootParameter[2].InitAsConstantBufferView(2);

  CD3DX12_DESCRIPTOR_RANGE textureRange;
  textureRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
  slotRootParameter[3].InitAsDescriptorTable(
      1, &textureRange, D3D12_SHADER_VISIBILITY_PIXEL);

  // 像素风纹理使用 point 过滤保持方块边缘，wrap 允许普通网格 UV 平铺。
  CD3DX12_STATIC_SAMPLER_DESC sampler(
      0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
      D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.0f,
      1, D3D12_COMPARISON_FUNC_ALWAYS,
      D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE, 0.0f, D3D12_FLOAT32_MAX,
      D3D12_SHADER_VISIBILITY_PIXEL);

  CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RD(
      4, slotRootParameter, 1, &sampler,
      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
