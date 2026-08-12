#include "Target/DirectX/DX12SceneTarget.h"

#include "Core/Window.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"
#include "Util/Color.h"
#include "d3dx12.h"

#include <algorithm>

using namespace z8;

void DX12SceneTarget::InitDescriptor() {
  D3D12_DESCRIPTOR_HEAP_DESC description{};
  description.NumDescriptors = 1;
  description.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  description.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  Ok(Ctx->Device->CreateDescriptorHeap(
      &description, IID_PPV_ARGS(DescriptorHeap.GetAddressOf())));
  Descriptor = DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

void DX12SceneTarget::InitBuffer() {
  const auto width = static_cast<UINT64>((std::max)(1, Render->GetWindow()->Width));
  const auto height = static_cast<UINT>((std::max)(1, Render->GetWindow()->Height));
  const auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  const auto description = CD3DX12_RESOURCE_DESC::Tex2D(
      Format, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
  D3D12_CLEAR_VALUE clear{};
  clear.Format = Format;
  clear.Color[0] = Color::EditorBackground.x;
  clear.Color[1] = Color::EditorBackground.y;
  clear.Color[2] = Color::EditorBackground.z;
  clear.Color[3] = Color::EditorBackground.w;
  Ok(Ctx->Device->CreateCommittedResource(
      &heap, D3D12_HEAP_FLAG_NONE, &description,
      D3D12_RESOURCE_STATE_RENDER_TARGET, &clear,
      IID_PPV_ARGS(Buffer.GetAddressOf())));
  Ctx->Device->CreateRenderTargetView(Buffer.Get(), nullptr, Descriptor);
  State = D3D12_RESOURCE_STATE_RENDER_TARGET;
}

void DX12SceneTarget::ResetBuffer() { Buffer.Reset(); }

void DX12SceneTarget::Bind() const {
  Render->Cmd.List->OMSetRenderTargets(1, &Descriptor, true,
                                       &Render->DepthStencil.Dpt);
}

void DX12SceneTarget::Clear() const {
  Render->Cmd.List->ClearRenderTargetView(Descriptor, Color::Clear, 0, nullptr);
}

void DX12SceneTarget::Transition(D3D12_RESOURCE_STATES targetState) {
  if (!Buffer || State == targetState)
    return;
  const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      Buffer.Get(), State, targetState);
  Render->Cmd.List->ResourceBarrier(1, &barrier);
  State = targetState;
}
