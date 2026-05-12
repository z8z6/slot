//
// Created by zhou_zhengming on 2026/5/12.
//

#include <d3d12.h>
#include "Target/DirectX/DX12Command.h"
#include "Target/DirectX/DX12Context.h"

void z8::DX12Command::Init()
{
  D3D12_COMMAND_QUEUE_DESC CD = {};
  CD.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  CD.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  Ok(Ctx->Device->CreateCommandQueue(&CD, IID_PPV_ARGS(&CmdQueue)));
  Ok(Ctx->Device->CreateCommandAllocator(
          D3D12_COMMAND_LIST_TYPE_DIRECT,
          IID_PPV_ARGS(CmdAllocator.GetAddressOf())));
  Ok(Ctx->Device->CreateCommandList(
          0,
          D3D12_COMMAND_LIST_TYPE_DIRECT,
          CmdAllocator.Get(),
          nullptr,
          IID_PPV_ARGS(CmdList.GetAddressOf())));
  Ok(CmdList->Close());
}

void z8::DX12Command::Reset()
{
  Ok(CmdList->Reset(CmdAllocator.Get(), nullptr));
}

void z8::DX12Command::Exec()
{
  Ok(CmdList->Close());

  // 执行渲染命令
  ID3D12CommandList* cmdsLists[] = { CmdList.Get() };
  CmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}
