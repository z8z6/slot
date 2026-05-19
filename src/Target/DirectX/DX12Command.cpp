//
// Created by zhou_zhengming on 2026/5/12.
//

#include <d3d12.h>
#include "Target/DirectX/DX12Command.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"

z8::DX12Command::DX12Command(DX12Render* R) : DX12Common(R)
{
}

z8::DX12Command::~DX12Command()
{
  Synchronize();
}

void z8::DX12Command::Init()
{
  D3D12_COMMAND_QUEUE_DESC CD = {};
  CD.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  CD.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  Ok(Ctx->Device->CreateCommandQueue(&CD, IID_PPV_ARGS(&Queue)));
  Ok(Ctx->Device->CreateCommandAllocator(
          D3D12_COMMAND_LIST_TYPE_DIRECT,
          IID_PPV_ARGS(Allocator.GetAddressOf())));
  Ok(Ctx->Device->CreateCommandList(
          0,
          D3D12_COMMAND_LIST_TYPE_DIRECT,
          Allocator.Get(),
          nullptr,
          IID_PPV_ARGS(List.GetAddressOf())));
  Ok(List->Close());

  Ok(Ctx->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
}

void z8::DX12Command::Synchronize()
{
  ++CurFence;
  Ok(Queue->Signal(Fence.Get(), CurFence));
  if (Fence->GetCompletedValue() >= CurFence) return;

  HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
  Ok(Fence->SetEventOnCompletion(CurFence, eventHandle));
  WaitForSingleObject(eventHandle, INFINITE);
  CloseHandle(eventHandle);
}

void z8::DX12Command::Reset()
{
  Ok(List->Reset(Allocator.Get(), nullptr));
}

void z8::DX12Command::ResetWithPso()
{
  Ok(List->Reset(Allocator.Get(), Render->PSO.Pipe.Get()));
}

void z8::DX12Command::CloseAndExecute()
{
  Ok(List->Close());

  // 执行渲染命令
  ID3D12CommandList* cmdsLists[] = { List.Get() };
  Queue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}
