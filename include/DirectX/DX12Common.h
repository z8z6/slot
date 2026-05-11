//
// Created by zhou_zhengming on 2026/5/9.
//
#pragma once

#include <wrl.h>
#include <cassert>

namespace z8 {
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
}

#define Ok(expr)              \
  {                           \
    HRESULT hr__ = (expr);    \
    assert(SUCCEEDED(hr__));  \
  }

