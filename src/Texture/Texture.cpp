#include "Texture/Texture.h"

#include <Windows.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <limits>

using Microsoft::WRL::ComPtr;
using namespace z8;

bool Texture::Load(const std::wstring& path, std::string* error) {
  const auto fail = [&](const char* message) {
    if (error) *error = message;
    Pixels.clear();
    Width = 0;
    Height = 0;
    return false;
  };

  // ResourceManager 可能在 UI 线程或测试线程构造；只在本函数成功初始化 COM 时
  // 配对释放，RPC_E_CHANGED_MODE 表示宿主已选择另一公寓模型，WIC 仍可直接使用。
  struct ComScope {
    bool Owns = false;
    ~ComScope() {
      if (Owns) CoUninitialize();
    }
  };
  // 守卫必须先于 WIC ComPtr 构造，使逆序析构时所有 WIC 对象先 Release。
  ComScope com{SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))};

  ComPtr<IWICImagingFactory> factory;
  ComPtr<IWICBitmapDecoder> decoder;
  ComPtr<IWICBitmapFrameDecode> frame;
  ComPtr<IWICFormatConverter> converter;
  HRESULT result = CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                                    CLSCTX_INPROC_SERVER,
                                    IID_PPV_ARGS(factory.GetAddressOf()));
  if (SUCCEEDED(result))
    result = factory->CreateDecoderFromFilename(
        path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad,
        decoder.GetAddressOf());
  if (SUCCEEDED(result)) result = decoder->GetFrame(0, frame.GetAddressOf());
  if (SUCCEEDED(result))
    result = factory->CreateFormatConverter(converter.GetAddressOf());
  if (SUCCEEDED(result))
    result = converter->Initialize(
        frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone,
        nullptr, 0.0, WICBitmapPaletteTypeCustom);
  if (SUCCEEDED(result)) result = converter->GetSize(&Width, &Height);

  const uint64_t byteCount = static_cast<uint64_t>(Width) * Height * 4;
  if (FAILED(result) || byteCount == 0 ||
      byteCount > std::numeric_limits<UINT>::max()) {
    return fail("Failed to decode texture as RGBA8.");
  }

  Pixels.resize(static_cast<size_t>(byteCount));
  result = converter->CopyPixels(
      nullptr, Width * 4, static_cast<UINT>(Pixels.size()),
      reinterpret_cast<BYTE*>(Pixels.data()));
  if (FAILED(result)) return fail("Failed to copy decoded texture pixels.");
  if (error) error->clear();
  return true;
}

bool Texture::Validate() const {
  return Width > 0 && Height > 0 &&
         Pixels.size() == static_cast<size_t>(Width) * Height * 4;
}
