#pragma once

#include "DX12Common.h"
#include "d3d12.h"

namespace z8 {

/**
 * 3D 场景专用离屏颜色缓冲。
 *
 * 它与交换链保持相同尺寸，SceneNode 只复制自己的矩形区域；这样停靠布局变化
 * 不会频繁重建 GPU 资源，也不要求 UI Shader 增加纹理采样 ABI。
 */
class DX12SceneTarget final : public DX12Common {
public:
  ComPtr<ID3D12Resource> Buffer;
  ComPtr<ID3D12DescriptorHeap> DescriptorHeap;
  D3D12_CPU_DESCRIPTOR_HANDLE Descriptor{};
  DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;

  explicit DX12SceneTarget(DX12Render *render) : DX12Common(render) {}

  void InitDescriptor();
  void InitBuffer();
  void ResetBuffer();
  void Bind() const;
  void Clear() const;
  /** 维护实际资源状态，避免离屏绘制与复制之间出现隐式状态假设。 */
  void Transition(D3D12_RESOURCE_STATES targetState);

private:
  D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_RENDER_TARGET;
};

} // namespace z8
