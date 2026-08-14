#pragma once

#include "DX12Common.h"
#include <d3d12.h>

namespace z8 {

/**
 * 单个渲染表面的深度模板目标。
 *
 * 尺寸与采样参数由调用方显式提供，因此主窗口和包含 SceneNode 的 Floating
 * surface 可以复用相同实现；该资源不读取 Application 主窗口几何。
 */
class DX12DepthStencil : public DX12Common {
public:
  ComPtr<ID3D12Resource> Buffer;
  ComPtr<ID3D12DescriptorHeap> DptHeap;
  D3D12_CPU_DESCRIPTOR_HANDLE Dpt{};
  DXGI_FORMAT Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

  explicit DX12DepthStencil(DX12Render *render) : DX12Common(render) {}

  void ClearBuffer() const;
  void InitBuffer(int width, int height, unsigned sampleCount,
                  unsigned sampleQuality);
  void InitDescriptor();
  void ResetBuffer();
};

} // namespace z8

