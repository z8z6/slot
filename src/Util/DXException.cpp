//
// Created by zhou_zhengming on 2026/5/11.
//

#include "Util/DXException.h"
#include <comdef.h>

using namespace z8;

DXException::DXException(HRESULT hr) : ErrorCode(hr) {
}
DXException::DXException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber) :
    ErrorCode(hr),
    FunctionName(functionName),
    Filename(filename),
    LineNumber(lineNumber)
{
}

std::wstring DXException::toString() const
{
  // Get the string description of the error code.
  _com_error err(ErrorCode);
  std::wstring msg = err.ErrorMessage();

  return FunctionName + L" failed in " + Filename + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}


