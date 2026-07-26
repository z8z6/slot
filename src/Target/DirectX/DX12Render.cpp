//
// Created by zhou_zhengming on 2026/5/8.
//

#include "Target/DirectX/DX12Render.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Shader.h"
#include "UI/Object/GameObject.h"
#include "Util/Math.h"
#include "d3dcompiler.h"
#include "d3dx12.h"
#include <dxgi1_4.h>

#include "UI/Mesh/Mesh.h"
#include "UI/Object/Camera.h"

using namespace DirectX;
using namespace z8;

z8::DX12Render::DX12Render(Application* app)
    : App(app), Cmd(this), SwapChain(this), Msaa(this), PSO(this), RootSignature(this),
      DepthStencil(this), RenderTarget(this), ConstBuffer(this), MeshManager(this), MaterialManager(this) {
  Ctx = &DX12Device::Instance();
}

void z8::DX12Render::Init()
{
  Msaa.Init();
  Cmd.Init();
  SwapChain.Init();
  RenderTarget.InitDescriptor();
  DepthStencil.InitDescriptor();
  ConstBuffer.InitDescriptor();

  Resize();

  Cmd.Reset();

  ConstBuffer.InitBuffer();
  RootSignature.Init();
  MeshManager.Init();
  MaterialManager.Init();
  InitObject();
  PSO.Init();

  Cmd.CloseAndExecute();
  Cmd.Synchronize();
}

void z8::DX12Render::Update()
{
  // 1. 更新相机坐标
  GetCamera()->Update(GetTimer());
  // 2. 更新全局常量
  GlobalConst.Update(this);
  // 3. 更新物体数据
  for (auto& O : RenderObjects) {
    O.Object->Update(GetTimer());
    memcpy(ConstBuffer.GetCPUOffset(O.ConstBufIndex), O.Object->ConstBuf(), O.Object->ConstBufSize());
  }
}

void z8::DX12Render::Draw()
{
  Ok(Cmd.Allocator->Reset());
  // 绑定渲染流水线
  Cmd.ResetWithPso();

  RenderTarget.Transition(false);

  // This needs to be reset whenever the command list is reset.
  Cmd.List->RSSetViewports(1, &ScreenView);
  Cmd.List->RSSetScissorRects(1, &ScissorRect);

  RenderTarget.Swap();
  RenderTarget.ClearBuffer();
  DepthStencil.ClearBuffer();

  RenderTarget.Bind();

  // 设置常量缓冲区的描述符堆
  ID3D12DescriptorHeap* descriptorHeaps[] = {ConstBuffer.DptHeap.Get()};
  Cmd.List->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

  RootSignature.Bind();

  // 寄存器 b1: 物体材质常量
  Cmd.List->SetGraphicsRootConstantBufferView(1, MaterialManager.Buffer.DefaultBuffer->GetGPUVirtualAddress());
  // 寄存器 b2: 全局常量
  Cmd.List->SetGraphicsRootDescriptorTable(2, ConstBuffer.GetGPUDescriptor(DX12GlobalConst::Index));

  // 依次绘制各个物体
  for (auto& O : RenderObjects) {
    MeshManager.Bind();
    // 寄存器 b0: 物体位置常量
    Cmd.List->SetGraphicsRootDescriptorTable(0, ConstBuffer.GetGPUDescriptor(O.ConstBufIndex));
    Cmd.List->DrawIndexedInstanced(O.SubMesh->IndexCount,
    1, O.SubMesh->StartIndexLocation,
    O.SubMesh->BaseVertexLocation, 0);
  }

  RenderTarget.Transition();

  Cmd.CloseAndExecute();
  SwapChain.Present();
  Cmd.Synchronize();
}

void DX12Render::Resize()
{
  Cmd.Synchronize();
  Cmd.Reset();

  RenderTarget.ResetBuffer();
  DepthStencil.ResetBuffer();
  SwapChain.Resize();
  RenderTarget.InitBuffer();
  DepthStencil.InitBuffer();

  Cmd.CloseAndExecute();
  Cmd.Synchronize();

  ScreenView.TopLeftX = 0;
  ScreenView.TopLeftY = 0;
  ScreenView.Width = static_cast<float>(GetWindow()->Width);
  ScreenView.Height = static_cast<float>(GetWindow()->Height);
  ScreenView.MinDepth = 0.0f;
  ScreenView.MaxDepth = 1.0f;

  ScissorRect = {0, 0, GetWindow()->Width, GetWindow()->Height};

  GetCamera()->UpdateProj(GetWindow()->AspectRatio());
}

void DX12Render::InitObject() {
  unsigned i = 0;
  for (auto* O : App->Objects) {
    DX12RenderObject RO(O);
    RO.ConstBufIndex = i;
    RO.SubMesh = MeshManager.GetSubMesh(O->Mesh);
    RenderObjects.push_back(RO);
    ++i;
  }
}

Camera * DX12Render::GetCamera() const
{
  return App->Camera;
}

Window* DX12Render::GetWindow() const {
  return &App->Window;
}

Timer *DX12Render::GetTimer() const {
  return &App->Timer;
}
Light *DX12Render::GetLight() const {
  return App->Light;
}
