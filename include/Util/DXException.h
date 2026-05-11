//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once
#include <intsafe.h>
#include <string>

namespace z8 {
class DXException
{
public:
  DXException() = default;
  DXException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

  std::wstring toString()const;

  HRESULT ErrorCode = S_OK;
  std::wstring FunctionName;
  std::wstring Filename;
  int LineNumber = -1;
};
}
