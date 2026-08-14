#pragma once

#include "Target/DirectX/DX12Common.h"
#include "Target/DirectX/DX12SwapChain.h"

#include <DirectXMath.h>
#include <d3d12.h>

namespace z8 {

/**
 * 单个 WindowSurface 的颜色目标集合。
 *
 * 后备缓冲由传入的交换链拥有，本类只保持 COM 引用、RTV 和可选 MSAA 颜色
 * 纹理；尺寸、采样配置和当前索引均由调用方显式提交，不读取主窗口状态。
 */
class DX12RenderTarget : public DX12Common {
public:
  ComPtr<ID3D12Resource> Buffer[DX12SwapChain::BufferCount];
  ComPtr<ID3D12Resource> MsaaBuffer;
  ComPtr<ID3D12DescriptorHeap> RtvHeap;
  D3D12_CPU_DESCRIPTOR_HANDLE CurrentRtv{};
  D3D12_CPU_DESCRIPTOR_HANDLE MsaaRtv{};
  DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  int CurrentBufferIndex = 0;

private:
  DirectX::XMFLOAT4 ClearColor{};
  unsigned RtvSize = 0;
  unsigned SampleCount = 1;
  unsigned SampleQuality = 0;

public:
  explicit DX12RenderTarget(DX12Render *render) : DX12Common(render) {}

  void Bind(const D3D12_CPU_DESCRIPTOR_HANDLE *depth = nullptr) const;
  void ClearBuffer() const;
  ID3D12Resource *GetBuffer() const;
  void InitBuffer(DX12SwapChain &swapChain, int width, int height,
                  unsigned sampleCount, unsigned sampleQuality,
                  const DirectX::XMFLOAT4 &clearColor);
  void InitDescriptor();
  bool IsMultisampled() const { return SampleCount > 1; }
  void ResetBuffer();
  void Resolve() const;
  void SelectBuffer(int index);
  void Transition(bool toPresent = true) const;
};

} // namespace z8
