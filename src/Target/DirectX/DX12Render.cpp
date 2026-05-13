//
// Created by zhou_zhengming on 2026/5/8.
//

#include "Target/DirectX/DX12Render.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include "UI/Object/IObject.h"
#include "Target/DirectX/DX12Context.h"
#include "Target/DirectX/DX12Shader.h"
#include "Util/Math.h"
#include "d3dcompiler.h"
#include "d3dx12.h"

#include <DirectXColors.h>
#include <dxgi1_4.h>

#include "UI/Material/IMaterial.h"
#include "UI/Mesh/IMesh.h"

using namespace DirectX;
using namespace z8;

z8::DX12Render::DX12Render(Application *app) : App(app) {
  Ctx = &DX12Context::Instance();
  Wnd = &App->Window;
  O = App->Objects[0];
  assert(O);
}

DX12Render::~DX12Render() {
  CmdSync();
  ConstBufGPU->Unmap(0, nullptr);
}

void z8::DX12Render::Init() {
  Ok(Ctx->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

  CreateMsaa();
  CreateCmd();
  CreateSwapChain();
  CreateDptHeap();
  Resize();

  CmdBegin();
  CreateCbv();
  CreateRootSignature();
  CreateMesh();
  CreateMeshView();
  CreateShader();
  CreatePSO();
  CmdEnd();
  CmdSync();
}

void z8::DX12Render::Update() {
  O->Update(mProj);
  memcpy(&ConstBufCPU[0], O->ConstBuf(), O->ConstBufSize());
}

void z8::DX12Render::Draw() {

  Ok(CmdAllocator->Reset());
  // 这里需要绑定渲染流水线
  Ok(CmdList->Reset(CmdAllocator.Get(), PSO.Get()));
  //CmdBegin();

  // Rtv 资源类型转换
  auto RenderBarrier = CD3DX12_RESOURCE_BARRIER::Transition(GetCurRtvBuf(),
          D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
  CmdList->ResourceBarrier(1, &RenderBarrier);

  // Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
  CmdList->RSSetViewports(1, &ScreenView);
  CmdList->RSSetScissorRects(1, &ScissorRect);

  // 清空缓冲区
  CreateDpt();
  CmdList->ClearRenderTargetView(RtvDpt, Colors::LightSteelBlue, 0, nullptr);
  CmdList->ClearDepthStencilView(DsvDpt, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

  // 设置要写入的缓冲区
  CmdList->OMSetRenderTargets(1, &RtvDpt,
    true, &DsvDpt);

  ID3D12DescriptorHeap* descriptorHeaps[] = { CbvDptHeap.Get() };
  CmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

  CmdList->SetGraphicsRootSignature(RootSignature.Get());

  // 指定顶点缓冲区和着色器
  CmdList->IASetVertexBuffers(0, 1, &Vv);
  CmdList->IASetIndexBuffer(&Iv);
  CmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  CmdList->SetGraphicsRootDescriptorTable(0, CbvDptHeap->GetGPUDescriptorHandleForHeapStart());

  // 绘制图形
  CmdList->DrawIndexedInstanced(O->Mesh->ICount(), 1, 0, 0, 0);

  // Rtv 资源类型转换
  auto PresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(GetCurRtvBuf(),
          D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  CmdList->ResourceBarrier(1, &PresentBarrier);

  CmdEnd();

  // 呈现当前缓冲区
  Ok(SwapChain->Present(0, 0));
  // 切换缓冲区
  CurRtvId = ++CurRtvId % RtvBufCount;

  // 等待命令执行完毕
  CmdSync();
}

void z8::DX12Render::CmdSync() {
  ++CurFence;
  Ok(CmdQueue->Signal(Fence.Get(), CurFence));
  if(Fence->GetCompletedValue() >= CurFence) return;

  HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
  Ok(Fence->SetEventOnCompletion(CurFence, eventHandle));
  WaitForSingleObject(eventHandle, INFINITE);
  CloseHandle(eventHandle);
}

void DX12Render::CmdBegin() {
  Ok(CmdList->Reset(CmdAllocator.Get(), nullptr));
}

void DX12Render::CmdEnd() {
  Ok(CmdList->Close());

  // 执行渲染命令
  ID3D12CommandList* cmdsLists[] = { CmdList.Get() };
  CmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}

// 查询 Msaa
// 初始化时调用一次
void DX12Render::CreateMsaa() {
  // 查询 MSAA 的支持情况
  D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
  msQualityLevels.Format = FormatRtv;
  msQualityLevels.SampleCount = 4;
  msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
  msQualityLevels.NumQualityLevels = 0;
  Ok(Ctx->Device->CheckFeatureSupport(
          D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
          &msQualityLevels,
          sizeof(msQualityLevels)));

  MsaaQuality = msQualityLevels.NumQualityLevels;
  assert(MsaaQuality > 0 && "Unexpected MSAA quality level.");
}

// 创建命令队列
// 初始化时调用一次
void DX12Render::CreateCmd() {
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

void DX12Render::Resize() {
  CmdSync();
  CmdBegin();

  for (auto & i : RtvBuf)
    i.Reset();
  DsvBuf.Reset();

  // 调整 SwapChain 大小
  Ok(SwapChain->ResizeBuffers(
	      RtvBufCount, Wnd->Width, Wnd->Height,
	      FormatRtv, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

  CreateRtv();
  CreateDsv();
  CmdEnd();
  CmdSync();

  ScreenView.TopLeftX = 0;
  ScreenView.TopLeftY = 0;
  ScreenView.Width    = static_cast<float>(Wnd->Width);
  ScreenView.Height   = static_cast<float>(Wnd->Height);
  ScreenView.MinDepth = 0.0f;
  ScreenView.MaxDepth = 1.0f;

  ScissorRect = {0, 0, Wnd->Width, Wnd->Height};

  XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f* Math::PI, Wnd->AspectRatio(), 1.0f, 1000.0f);
  XMStoreFloat4x4(&mProj, P);
}

// 创建交换链
// 初始化时调用一次
void z8::DX12Render::CreateSwapChain() {
  SwapChain.Reset();
  DXGI_SWAP_CHAIN_DESC SD;
  SD.BufferDesc.Width = Wnd->Width;
  SD.BufferDesc.Height = Wnd->Height;
  SD.BufferDesc.RefreshRate.Numerator = 60;
  SD.BufferDesc.RefreshRate.Denominator = 1;
  SD.BufferDesc.Format = FormatRtv;
  SD.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  SD.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  SD.SampleDesc.Count = EnableMsaa ? 4 : 1;
  SD.SampleDesc.Quality = EnableMsaa ? (MsaaQuality - 1) : 0;
  SD.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  SD.BufferCount = RtvBufCount;
  SD.OutputWindow = Wnd->Wnd;
  SD.Windowed = true;
  SD.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  SD.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  // SwapChain 创建依赖 CmdQueue
  Ok(Ctx->Factory->CreateSwapChain(CmdQueue.Get(), &SD, SwapChain.GetAddressOf()));
}

// 创建资源描述符
// 每次缓冲区交换时调用一次
void z8::DX12Render::CreateDpt() {
  // 只需调用一次
  DsvDpt = DsvDptHeap->GetCPUDescriptorHandleForHeapStart();
  // 计算描述符的偏移
  RtvDpt = CD3DX12_CPU_DESCRIPTOR_HANDLE(
          RtvDptHeap->GetCPUDescriptorHandleForHeapStart(),
          CurRtvId,
          RtvDptSize);
}

// 创建 Dsv 缓冲区，并绑定描述符
// 每次 Resize 时调用一次
void DX12Render::CreateDsv() {

  // 创建 Dsv 缓冲区
  D3D12_RESOURCE_DESC BD;
  BD.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  BD.Alignment = 0;
  BD.Width = Wnd->Width;
  BD.Height = Wnd->Height;
  BD.DepthOrArraySize = 1;
  BD.MipLevels = 1;
  BD.Format = DXGI_FORMAT_R24G8_TYPELESS;
  BD.SampleDesc.Count = EnableMsaa ? 4 : 1;
  BD.SampleDesc.Quality = EnableMsaa ? (MsaaQuality - 1) : 0;
  BD.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  BD.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

  D3D12_CLEAR_VALUE Clv;
  Clv.Format = FormatDsv;
  Clv.DepthStencil.Depth = 1.0f;
  Clv.DepthStencil.Stencil = 0;

  auto HP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  Ok(Ctx->Device->CreateCommittedResource(
      &HP,
      D3D12_HEAP_FLAG_NONE,
      &BD,
      D3D12_RESOURCE_STATE_COMMON,
      &Clv,
      IID_PPV_ARGS(DsvBuf.GetAddressOf())));

  // 绑定 Dsv 描述符
  D3D12_DEPTH_STENCIL_VIEW_DESC DD;
  DD.Flags = D3D12_DSV_FLAG_NONE;
  DD.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  DD.Format = FormatDsv;
  DD.Texture2D.MipSlice = 0;
  Ctx->Device->CreateDepthStencilView(DsvBuf.Get(), &DD, DsvDptHeap->GetCPUDescriptorHandleForHeapStart());

  // 状态转换
  auto WriteBarrier = CD3DX12_RESOURCE_BARRIER::Transition(DsvBuf.Get(),
              D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
  CmdList->ResourceBarrier(1, &WriteBarrier);
}

// 创建 Rtv 缓冲区，并绑定描述符
// 每次 Resize 时调用一次
void DX12Render::CreateRtv() {
  CurRtvId = 0;
  CD3DX12_CPU_DESCRIPTOR_HANDLE Dpt(RtvDptHeap->GetCPUDescriptorHandleForHeapStart());
  for (UINT i = 0; i < RtvBufCount; i++)
  {
    Ok(SwapChain->GetBuffer(i, IID_PPV_ARGS(&RtvBuf[i])));
    Ctx->Device->CreateRenderTargetView(RtvBuf[i].Get(), nullptr, Dpt);
    Dpt.Offset(1, RtvDptSize);
  }
}

void DX12Render::CreateCbv() {

  unsigned ByteSize = (sizeof (XMFLOAT4X4) + 255) & ~255;

  auto Prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  auto D = CD3DX12_RESOURCE_DESC::Buffer(ByteSize);
  Ok(Ctx->Device->CreateCommittedResource(
     &Prop, D3D12_HEAP_FLAG_NONE,
     &D, D3D12_RESOURCE_STATE_GENERIC_READ,
     nullptr,
     IID_PPV_ARGS(&ConstBufGPU)));

  Ok(ConstBufGPU->Map(0, nullptr, reinterpret_cast<void**>(&ConstBufCPU)));

  D3D12_GPU_VIRTUAL_ADDRESS cbAddress = ConstBufGPU->GetGPUVirtualAddress();
  // Offset to the ith object constant buffer in the buffer.
  int boxCBufIndex = 0;
  cbAddress += boxCBufIndex * ByteSize;

  D3D12_CONSTANT_BUFFER_VIEW_DESC CD;
  CD.BufferLocation = cbAddress;
  CD.SizeInBytes = ByteSize;

  Ctx->Device->CreateConstantBufferView(&CD, CbvDptHeap->GetCPUDescriptorHandleForHeapStart());
}

// 创建资源描述符堆，存放描述符
// 初始化时调用一次
void DX12Render::CreateDptHeap() {
  // 查询堆描述符的大小
  RtvDptSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  DsvDptSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
  CbvSrvUavDptSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  // Rtv 的描述符堆
  D3D12_DESCRIPTOR_HEAP_DESC RD;
  RD.NumDescriptors = RtvBufCount;  // 2个描述符
  RD.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  RD.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  RD.NodeMask = 0;
  Ok(Ctx->Device->CreateDescriptorHeap(&RD, IID_PPV_ARGS(RtvDptHeap.GetAddressOf())));

  // Dsv 的描述符堆
  D3D12_DESCRIPTOR_HEAP_DESC DD;
  DD.NumDescriptors = 1;
  DD.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  DD.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  DD.NodeMask = 0;
  Ok(Ctx->Device->CreateDescriptorHeap(&DD, IID_PPV_ARGS(DsvDptHeap.GetAddressOf())));

  // Cbv 的描述符堆
  D3D12_DESCRIPTOR_HEAP_DESC CD;
  CD.NumDescriptors = 1;
  CD.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  CD.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  CD.NodeMask = 0;
  Ok(Ctx->Device->CreateDescriptorHeap(&CD, IID_PPV_ARGS(CbvDptHeap.GetAddressOf())));

  // 对于 ComPtr, operator &() = ReleaseAndGetAddressOf()
}

void DX12Render::CreateRootSignature() {
  // Root parameter can be a table, root descriptor or root constants.
  CD3DX12_ROOT_PARAMETER slotRootParameter[1];

  // Create a single descriptor table of CBVs.
  CD3DX12_DESCRIPTOR_RANGE cbvTable;
  cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
  slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

  // A root signature is an array of root parameters.
  CD3DX12_ROOT_SIGNATURE_DESC RD(1, slotRootParameter, 0, nullptr,
          D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
  ComPtr<ID3DBlob> serializedRootSig = nullptr;
  ComPtr<ID3DBlob> errorBlob = nullptr;
  Ok(D3D12SerializeRootSignature(&RD, D3D_ROOT_SIGNATURE_VERSION_1,
          serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf()));

  if(errorBlob != nullptr)
  {
    ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
  }

  Ok(Ctx->Device->CreateRootSignature(
          0,
          serializedRootSig->GetBufferPointer(),
          serializedRootSig->GetBufferSize(),
          IID_PPV_ARGS(RootSignature.GetAddressOf())));
}

ID3D12Resource *z8::DX12Render::GetCurRtvBuf() const {
  return RtvBuf[CurRtvId].Get();
}

void DX12Render::CreateMesh() {
  Ok(D3DCreateBlob(O->Mesh->VSize(), &VBufCPU));
  CopyMemory(VBufCPU->GetBufferPointer(), O->Mesh->V.data(), O->Mesh->VSize());

  Ok(D3DCreateBlob(O->Mesh->ISize(), &IBufCPU));
  CopyMemory(IBufCPU->GetBufferPointer(), O->Mesh->I.data(), O->Mesh->ISize());

  VBufGPU = CreateDefaultBuffer(O->Mesh->V.data(), O->Mesh->VSize(), VBufUpload);

  IBufGPU = CreateDefaultBuffer(O->Mesh->I.data(), O->Mesh->ISize(), IBufUpload);
}

void DX12Render::CreateMeshView() {
  Vv.BufferLocation = VBufGPU->GetGPUVirtualAddress();
  Vv.StrideInBytes = O->Mesh->VElemSize();
  Vv.SizeInBytes = O->Mesh->VSize();

  Iv.BufferLocation = IBufGPU->GetGPUVirtualAddress();
  Iv.Format = FormatIBuf;
  Iv.SizeInBytes = O->Mesh->ISize();
}

void DX12Render::CreateShader() {

  InputLayout =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
  };
}

void DX12Render::CreatePSO() {
  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
  ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
  psoDesc.InputLayout = { InputLayout.data(), static_cast<UINT>(InputLayout.size()) };
  psoDesc.pRootSignature = RootSignature.Get();
  psoDesc.VS =
      {
        static_cast<BYTE*>(O->Material->V->ByteCode->GetBufferPointer()),
        O->Material->V->ByteCode->GetBufferSize()
};
  psoDesc.PS =
      {
        static_cast<BYTE*>(O->Material->P->ByteCode->GetBufferPointer()),
        O->Material->P->ByteCode->GetBufferSize()
};
  psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
  psoDesc.SampleMask = UINT_MAX;
  psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  psoDesc.NumRenderTargets = 1;
  psoDesc.RTVFormats[0] = FormatRtv;
  psoDesc.SampleDesc.Count = EnableMsaa ? 4 : 1;
  psoDesc.SampleDesc.Quality = EnableMsaa ? (MsaaQuality - 1) : 0;
  psoDesc.DSVFormat = FormatDsv;
  Ok(Ctx->Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PSO)));
}

ComPtr<ID3D12Resource> DX12Render::CreateDefaultBuffer(const void* initData,
    UINT64 byteSize,
    ComPtr<ID3D12Resource>& uploadBuffer) {
  ComPtr<ID3D12Resource> defaultBuffer;

  // Create the actual default buffer resource.
  auto D = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
  auto Prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  Ok(Ctx->Device->CreateCommittedResource(
      &Prop, D3D12_HEAP_FLAG_NONE,
      &D, D3D12_RESOURCE_STATE_COMMON,
      nullptr,
      IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

  Prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  Ok(Ctx->Device->CreateCommittedResource(
      &Prop, D3D12_HEAP_FLAG_NONE,
      &D, D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(uploadBuffer.GetAddressOf())));

  // Describe the data we want to copy into the default buffer.
  D3D12_SUBRESOURCE_DATA subResourceData = {};
  subResourceData.pData = initData;
  subResourceData.RowPitch = byteSize;
  subResourceData.SlicePitch = subResourceData.RowPitch;

  // Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
  // will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
  // the intermediate upload heap data will be copied to mBuffer.
  auto Barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
              D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
  CmdList->ResourceBarrier(1, &Barrier);
  UpdateSubresources<1>(CmdList.Get(), defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
  auto Barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
              D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
  CmdList->ResourceBarrier(1, &Barrier1);

  // Note: uploadBuffer has to be kept alive after the above function calls because
  // the command list has not been executed yet that performs the actual copy.
  // The caller can Release the uploadBuffer after it knows the copy has been executed.
  return defaultBuffer;
}
