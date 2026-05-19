//
// Created by zhou_zhengming on 2026/5/19.
//

#include "Util/Error.h"
#include "Target/DirectX/DX12Device.h"
#include <comdef.h>
#include <d3d12.h>
#include <iostream>
#include <ostream>

z8::Error::Error(HRESULT hr) : ErrorCode(hr) { Ctx = &DX12Device::Instance(); }

void z8::Error::PrintError() const {
  _com_error e0(ErrorCode);
  std::wstring msg0 = e0.ErrorMessage();
  std::wcout << msg0 << std::endl;
  MessageBoxW(nullptr, msg0.c_str(), L"Reason 0", MB_OK);

  HRESULT hr = Ctx->Device->GetDeviceRemovedReason();
  _com_error e1(hr);
  std::wstring msg1 = e1.ErrorMessage();
  std::wcout << msg1 << std::endl;
  MessageBoxW(nullptr, msg1.c_str(), L"Reason 1", MB_OK);
}