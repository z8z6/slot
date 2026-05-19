//
// Created by zhou_zhengming on 2026/5/17.
//

#pragma once
#include <dxgiformat.h>
#include "DX12Common.h"

namespace z8
{
class DX12Msaa : public DX12Common {
public:
  bool EnableMsaa = false;
  unsigned MsaaQuality = 0;
  unsigned SampleCount = 4;
  DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;

  DX12Msaa(DX12Render* R) : DX12Common(R){}
  void Init();
};
}
