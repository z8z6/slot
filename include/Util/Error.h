//
// Created by zhou_zhengming on 2026/5/19.
//
#pragma once
#include <intsafe.h>
#include <string>

namespace z8 {
class DX12Device;
class Error {
public:
  HRESULT ErrorCode;
  DX12Device* Ctx;

  Error(HRESULT hr);
  void PrintError() const;
};
}


#define DebugError(expr)    \
{                           \
  HRESULT hr__ = (expr);    \
  if(FAILED(hr__)){         \
    z8::Error e(hr__);      \
    e.PrintError();         \
    assert(false);          \
  }                         \
}
