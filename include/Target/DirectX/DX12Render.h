//
// Created by zhou_zhengming on 2026/5/8.
//
#pragma once

#include "Target/IRender.h"
#include "DX12Common.h"
#include "d3d12.h"

#include <DirectXMath.h>
#include <cstdint>
#include <vector>

class IDXGISwapChain;

namespace z8 {
class Window;
class IObject;
class DX12Context;
class Application;

// 这个类是每个窗口独立的
class DX12Render : public IRender {
private:
  static const int RtvBufCount = 2;
  Application* App;
  Window* Wnd;
  DX12Context* Ctx;
  IObject* O;

  // Sync
  ComPtr<ID3D12Fence> Fence;
  UINT64 CurFence = 0;

  // Command
  ComPtr<ID3D12CommandQueue> CmdQueue;
  ComPtr<ID3D12CommandAllocator> CmdAllocator;
  ComPtr<ID3D12GraphicsCommandList> CmdList;

  // =============================================================== //
  // 当前 Rtv 缓冲区索引
  int CurRtvId = 0;
  ComPtr<IDXGISwapChain> SwapChain;
  // Rtv 缓冲区
  ComPtr<ID3D12Resource> RtvBuf[RtvBufCount];
  // Dsv 缓冲区
  ComPtr<ID3D12Resource> DsvBuf;
  // 缓冲区内存类型
  DXGI_FORMAT FormatRtv = DXGI_FORMAT_R8G8B8A8_UNORM;
  DXGI_FORMAT FormatDsv = DXGI_FORMAT_D24_UNORM_S8_UINT;

  // 资源描述符堆
  ComPtr<ID3D12DescriptorHeap> RtvDptHeap;
  ComPtr<ID3D12DescriptorHeap> DsvDptHeap;
  ComPtr<ID3D12DescriptorHeap> CbvDptHeap;
  // 单个描述符的大小
  UINT RtvDptSize = 0;
  UINT DsvDptSize = 0;
  UINT CbvSrvUavDptSize = 0;
  // 资源描述符
  D3D12_CPU_DESCRIPTOR_HANDLE RtvDpt;
  D3D12_CPU_DESCRIPTOR_HANDLE DsvDpt;

  // =============================================================== //
  // 顶点缓冲区和上传堆

  ComPtr<ID3DBlob> VBufCPU;
  ComPtr<ID3DBlob> IBufCPU;

  ComPtr<ID3D12Resource> VBufGPU;
  ComPtr<ID3D12Resource> IBufGPU;

  D3D12_VERTEX_BUFFER_VIEW Vv;
  D3D12_INDEX_BUFFER_VIEW Iv;

  ComPtr<ID3D12Resource> VBufUpload;
  ComPtr<ID3D12Resource> IBufUpload;

  DXGI_FORMAT FormatIBuf = DXGI_FORMAT_R16_UINT;

  ComPtr<ID3D12Resource> ConstBufGPU;
  char* ConstBufCPU;
  ComPtr<ID3D12RootSignature> RootSignature;

  // =============================================================== //
  // Shader

  std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
  ComPtr<ID3D12PipelineState> PSO;

  // =============================================================== //

  D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;

  D3D12_VIEWPORT ScreenView;
  D3D12_RECT ScissorRect;

  DirectX::XMFLOAT4X4 mProj;

  // MSAA
  bool EnableMsaa = false;
  UINT MsaaQuality = 0;

public:
  DX12Render(Application* app);
  ~DX12Render() override;

  void Init() override;
  void Update() override;
  void Draw() override;
  void Resize() override;

private:
  void CmdSync();
  void CmdBegin();
  void CmdEnd();
  void CreateMsaa();
  void CreateCmd();
  void CreateSwapChain();
  void CreateDptHeap();
  void CreateRootSignature();
  void CreateDpt();
  void CreateDsv();
  void CreateRtv();
  void CreateCbv();
  void CreateMesh();
  void CreateMeshView();
  void CreateShader();
  void CreatePSO();
  ComPtr<ID3D12Resource> CreateDefaultBuffer(const void* initData,
    uint64_t byteSize, ComPtr<ID3D12Resource>& uploadBuffer);
  ID3D12Resource* GetCurRtvBuf() const;
};

}




