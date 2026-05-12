//
// Created by zhou_zhengming on 2026/5/12.
//

#include "Target/DirectX/DX12Common.h"
#include "Target/DirectX/DX12Context.h"

z8::DX12Common::DX12Common()
{
  Ctx = &DX12Context::Instance();
}

z8::DX12Common::~DX12Common()
{
}
