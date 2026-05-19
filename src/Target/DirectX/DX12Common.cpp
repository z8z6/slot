//
// Created by zhou_zhengming on 2026/5/12.
//

#include "Target/DirectX/DX12Common.h"
#include "Target/DirectX/DX12Device.h"

z8::DX12Common::DX12Common(DX12Render* R) : Render(R)
{
  Ctx = &DX12Device::Instance();
}