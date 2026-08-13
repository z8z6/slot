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
  /** UI 与 3D 几何统一使用 4x MSAA；交换链仍保持 Flip 模型要求的单采样。 */
  bool EnableMsaa = true;
  unsigned MsaaQuality = 0;
  unsigned SampleCount = 4;
  DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;

  DX12Msaa(DX12Render* R) : DX12Common(R){}
  void Init();
  unsigned GetMsaaQuality() const;
  unsigned GetSampleCount() const;
};
} // namespace z8
