#pragma once

#include "Target/DirectX/DX12Common.h"

#include <d2d1_3.h>
#include <d3d11on12.h>
#include <dwrite.h>
#include <unordered_map>
#include <vector>

namespace z8::ui {
class Layout;
class TextNode;
}

namespace z8 {

/**
 * 在 DX12 UI 几何之后绘制 DirectWrite 文字的互操作通道。
 *
 * D3D11On12 只包装当前交换链缓冲，不拥有其生命周期；Resize 前必须释放包装，
 * 绘制后必须 Flush，保证 Direct2D 提交发生在 DX12 Present 之前。
 */
class DX12TextRenderer : public DX12Common {
public:
  explicit DX12TextRenderer(DX12Render *render);

  void Init();
  /** 为附加交换链建立独立 DirectWrite 包装目标。 */
  void Init(ID3D12Resource *const buffers[2], DXGI_FORMAT format);
  /** 按 Direct2D → wrapped resource → D3D11On12 的依赖顺序显式释放。 */
  void Shutdown();
  void PrepareResize();
  void Resize();
  void Draw(const ui::Layout &layout);
  void Draw(const std::vector<ui::TextNode *> &texts, int bufferIndex,
            float originX, float originY);

private:
  void CreateDevices();
  void CreateTargets(ID3D12Resource *const buffers[2], DXGI_FORMAT format);
  IDWriteTextFormat *GetFormat(const ui::TextNode &node);

  ComPtr<ID3D11Device> D3D11Device;
  ComPtr<ID3D11DeviceContext> D3D11Context;
  ComPtr<ID3D11On12Device> D3D11On12Device;
  ComPtr<ID2D1Factory3> D2DFactory;
  ComPtr<ID2D1Device2> D2DDevice;
  ComPtr<ID2D1DeviceContext2> D2DContext;
  ComPtr<IDWriteFactory> WriteFactory;
  ComPtr<ID3D11Resource> WrappedBuffers[2];
  ComPtr<ID2D1Bitmap1> Targets[2];
  std::unordered_map<unsigned, ComPtr<IDWriteTextFormat>> Formats;
  bool Initialized = false;
  ID3D12Resource *SourceBuffers[2]{};
  DXGI_FORMAT SourceFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
};

} // namespace z8
