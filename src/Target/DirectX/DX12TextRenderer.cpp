#include "Target/DirectX/DX12TextRenderer.h"

#include "Target/DirectX/DX12Command.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"
#include "UI/Layout/TextNode.h"

#include <algorithm>
#include <string>

using namespace z8;

namespace {

std::wstring ToWide(const std::string &text) {
  if (text.empty())
    return {};
  const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                       text.data(), static_cast<int>(text.size()),
                                       nullptr, 0);
  if (size <= 0)
    return L"\uFFFD";
  std::wstring result(static_cast<size_t>(size), L'\0');
  MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
                      static_cast<int>(text.size()), result.data(), size);
  return result;
}

} // namespace

DX12TextRenderer::DX12TextRenderer(DX12Render *render) : DX12Common(render) {}

void DX12TextRenderer::Init(ID3D12Resource *const buffers[2],
                            DXGI_FORMAT format) {
  CreateDevices();
  CreateTargets(buffers, format);
  Initialized = true;
}

void DX12TextRenderer::CreateDevices() {
  UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
  IUnknown *queues[] = {Render->Cmd.Queue.Get()};
  Ok(D3D11On12CreateDevice(Ctx->Device.Get(), flags, nullptr, 0, queues, 1, 0,
                           D3D11Device.GetAddressOf(),
                           D3D11Context.GetAddressOf(), nullptr));
  Ok(D3D11Device.As(&D3D11On12Device));
  Ok(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                       D2DFactory.GetAddressOf()));
  ComPtr<IDXGIDevice> dxgiDevice;
  Ok(D3D11On12Device.As(&dxgiDevice));
  Ok(D2DFactory->CreateDevice(dxgiDevice.Get(), D2DDevice.GetAddressOf()));
  Ok(D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
                                    D2DContext.GetAddressOf()));
  Ok(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                         reinterpret_cast<IUnknown **>(
                             WriteFactory.GetAddressOf())));
}

void DX12TextRenderer::Shutdown() {
  if (!Initialized)
    return;

  // DeviceContext 会持有当前 target；必须先解绑并清空 D3D11 延迟引用，之后才
  // 能释放由交换链后备缓冲创建的 wrapped resource。依赖逆序不能交给成员的
  // 默认析构顺序，否则 D3D11On12 会在退出阶段报告仍被引用的资源。
  D2DContext->SetTarget(nullptr);
  D3D11Context->ClearState();
  D3D11Context->Flush();
  for (auto &target : Targets)
    target.Reset();
  for (auto &buffer : WrappedBuffers)
    buffer.Reset();
  Formats.clear();
  WriteFactory.Reset();
  D2DContext.Reset();
  D2DDevice.Reset();
  D2DFactory.Reset();
  D3D11On12Device.Reset();
  D3D11Context.Reset();
  D3D11Device.Reset();
  Initialized = false;
}

void DX12TextRenderer::CreateTargets(ID3D12Resource *const buffers[2],
                                     DXGI_FORMAT format) {
  const auto properties = D2D1::BitmapProperties1(
      D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
      D2D1::PixelFormat(format,
                        D2D1_ALPHA_MODE_PREMULTIPLIED));
  for (int i = 0; i < 2; ++i) {
    D3D11_RESOURCE_FLAGS flags{};
    flags.BindFlags = D3D11_BIND_RENDER_TARGET;
    Ok(D3D11On12Device->CreateWrappedResource(
        buffers[i], &flags,
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT,
        IID_PPV_ARGS(WrappedBuffers[i].GetAddressOf())));
    ComPtr<IDXGISurface> surface;
    Ok(WrappedBuffers[i].As(&surface));
    Ok(D2DContext->CreateBitmapFromDxgiSurface(surface.Get(), &properties,
                                               Targets[i].GetAddressOf()));
  }
}

void DX12TextRenderer::PrepareResize() {
  // ResizeBuffers 要求不存在任何后备缓冲引用，包装资源必须在 DX12 buffer 前释放。
  if (!Initialized)
    return;
  D2DContext->SetTarget(nullptr);
  for (auto &target : Targets)
    target.Reset();
  for (auto &buffer : WrappedBuffers)
    buffer.Reset();
  D3D11Context->Flush();
}

void DX12TextRenderer::Resize(ID3D12Resource *const buffers[2],
                              DXGI_FORMAT format) {
  if (!Initialized)
    return;
  CreateTargets(buffers, format);
}

IDWriteTextFormat *DX12TextRenderer::GetFormat(const ui::TextNode &node) {
  const unsigned key = static_cast<unsigned>(node.FontSize * 64.0f) ^
                       static_cast<unsigned>(
                           std::hash<std::wstring>{}(node.FontFamily));
  auto iterator = Formats.find(key);
  if (iterator != Formats.end())
    return iterator->second.Get();
  ComPtr<IDWriteTextFormat> format;
  Ok(WriteFactory->CreateTextFormat(
      node.FontFamily.c_str(), nullptr, DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, node.FontSize,
      L"zh-cn", format.GetAddressOf()));
  format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  return Formats.emplace(key, std::move(format)).first->second.Get();
}

void DX12TextRenderer::Draw(const std::vector<ui::TextNode *> &texts,
                            int bufferIndex, float originX, float originY) {
  // 即使本帧没有文字也必须完成 wrapped-resource 的状态交接，使后备缓冲从
  // RENDER_TARGET 转为 PRESENT；资源状态不能依赖 UI 树是否恰好含有 TextNode。
  const int index = bufferIndex;
  ID3D11Resource *resource = WrappedBuffers[index].Get();
  D3D11On12Device->AcquireWrappedResources(&resource, 1);
  D2DContext->SetTarget(Targets[index].Get());
  D2DContext->BeginDraw();
  for (const auto *node : texts) {
    if (!node->EffectiveVisible || node->Text.empty())
      continue;
    auto *format = GetFormat(*node);
    format->SetWordWrapping(node->Wrap ? DWRITE_WORD_WRAPPING_WRAP
                                       : DWRITE_WORD_WRAPPING_NO_WRAP);
    const auto alignment = node->Alignment == ui::TextAlignment::Center
                               ? DWRITE_TEXT_ALIGNMENT_CENTER
                           : node->Alignment == ui::TextAlignment::Trailing
                               ? DWRITE_TEXT_ALIGNMENT_TRAILING
                               : DWRITE_TEXT_ALIGNMENT_LEADING;
    format->SetTextAlignment(alignment);
    ComPtr<ID2D1SolidColorBrush> brush;
    const auto &color = node->Color;
    Ok(D2DContext->CreateSolidColorBrush(
        D2D1::ColorF(color.x, color.y, color.z, color.w),
        brush.GetAddressOf()));
    const auto text = ToWide(node->Text);
    const auto drawClip = [&](const DirectX::XMFLOAT4 &clip) {
      D2DContext->PushAxisAlignedClip(
          D2D1::RectF(clip.x - originX, clip.y - originY,
                      clip.z - originX, clip.w - originY),
          D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
      D2DContext->DrawTextW(text.c_str(), static_cast<UINT32>(text.size()),
                           format,
                           D2D1::RectF(node->Left - originX,
                                       node->Top - originY,
                                       node->Left + node->Width - originX,
                                       node->Top + node->Height - originY),
                           brush.Get());
      D2DContext->PopAxisAlignedClip();
    };
    if (node->HasTextOcclusion) {
      // Popup 完全覆盖文字时片段集合为空；此时不能退回原始 clip，否则底层
      // Panel 标题会再次穿透不透明菜单背景。
      for (const auto &clip : node->VisibleTextClips)
        drawClip(clip);
    } else {
      drawClip(node->VisibleClip);
    }
  }
  Ok(D2DContext->EndDraw());
  D3D11On12Device->ReleaseWrappedResources(&resource, 1);
  D3D11Context->Flush();
}
