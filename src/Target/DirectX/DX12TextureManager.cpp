#include "Target/DirectX/DX12TextureManager.h"

#include "Core/Application.h"
#include "Resource/ResourceManager.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"
#include "d3dx12.h"

using namespace z8;

DX12TextureManager::DX12TextureManager(DX12Render* render)
    : DX12Common(render) {}

void DX12TextureManager::Bind() const {
  if (!DescriptorHeap) return;
  ID3D12DescriptorHeap* heaps[] = {DescriptorHeap.Get()};
  Render->Cmd.List->SetDescriptorHeaps(_countof(heaps), heaps);
}

D3D12_GPU_DESCRIPTOR_HANDLE DX12TextureManager::GetGPUDescriptor(
    ResourceHandle<BaseTexture> texture) const {
  const auto iterator = Indices.find(texture);
  if (iterator == Indices.end() || !DescriptorHeap) return {};
  auto handle = DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
  handle.ptr += static_cast<uint64_t>(iterator->second) * DescriptorSize;
  return handle;
}

void DX12TextureManager::Init() {
  const auto& textures = Render->App->Resources.Textures;
  DescriptorHeap.Reset();
  Indices.clear();
  Resources.clear();
  Uploads.clear();
  if (textures.Size() == 0) return;

  D3D12_DESCRIPTOR_HEAP_DESC heapDescription{};
  heapDescription.NumDescriptors = static_cast<UINT>(textures.Size());
  heapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  heapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  Ok(Ctx->Device->CreateDescriptorHeap(
      &heapDescription, IID_PPV_ARGS(DescriptorHeap.GetAddressOf())));
  DescriptorSize = Ctx->Device->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  uint32_t index = 0;
  textures.Visit([&](ResourceHandle<BaseTexture> handle, const BaseTexture& texture) {
    auto description = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R8G8B8A8_UNORM, texture.Width, texture.Height);
    auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ComPtr<ID3D12Resource> resource;
    Ok(Ctx->Device->CreateCommittedResource(
        &defaultHeap, D3D12_HEAP_FLAG_NONE, &description,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
        IID_PPV_ARGS(resource.GetAddressOf())));

    const UINT64 uploadSize = GetRequiredIntermediateSize(resource.Get(), 0, 1);
    auto uploadDescription = CD3DX12_RESOURCE_DESC::Buffer(uploadSize);
    auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    ComPtr<ID3D12Resource> upload;
    Ok(Ctx->Device->CreateCommittedResource(
        &uploadHeap, D3D12_HEAP_FLAG_NONE, &uploadDescription,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(upload.GetAddressOf())));

    D3D12_SUBRESOURCE_DATA source{};
    source.pData = texture.Pixels.data();
    source.RowPitch = static_cast<LONG_PTR>(texture.Width) * 4;
    source.SlicePitch = source.RowPitch * texture.Height;
    // lambda 内显式经 this 解析同名 Render 成员，保持 C++17 兼容。
    UpdateSubresources(this->Render->Cmd.List.Get(), resource.Get(), upload.Get(),
                       0, 0, 1, &source);
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    this->Render->Cmd.List->ResourceBarrier(1, &barrier);

    D3D12_SHADER_RESOURCE_VIEW_DESC view{};
    view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    view.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    view.Texture2D.MipLevels = 1;
    auto cpu = DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += static_cast<uint64_t>(index) * DescriptorSize;
    Ctx->Device->CreateShaderResourceView(resource.Get(), &view, cpu);

    Indices.emplace(handle, index++);
    Resources.push_back(std::move(resource));
    Uploads.push_back(std::move(upload));
  });
}
