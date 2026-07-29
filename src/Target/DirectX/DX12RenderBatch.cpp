//
// Created by zhou_zhengming on 2026/7/29.
//

#include "Target/DirectX/DX12RenderBatch.h"
#include "Target/DirectX/DX12Render.h"

using namespace z8;

DX12RenderObject::DX12RenderObject(GameObject *O)
: Object(O), SubMesh(nullptr), ConstBufIndex(0) {

}

DX12RenderBatch::DX12RenderBatch(DX12Render *R)
    : DX12Common(R), Buffer(this), Pipe(R) {}

void DX12RenderBatch::Init(std::vector<GameObject *> &Os) {
  // 1. 初始化 RO
  unsigned i = 0;
  for (auto* O : Os) {
    DX12RenderObject RO(O);
    RO.ConstBufIndex = i;
    RO.SubMesh = Render->MeshManager.GetSubMesh(O->Mesh);
    ROs.push_back(RO);
    ++i;
  }

  // 2. 初始化常量缓冲区
  Buffer.InitDescriptor();
  Buffer.InitBuffer();
  Pipe.Init(Os[0]);
}

void DX12RenderBatch::Update() const {
  for (auto& O : ROs) {
    O.Object->Update(Render->GetTimer());
    memcpy(Buffer.GetCPUOffset(O.ConstBufIndex), O.Object->ConstBuf(), O.Object->ConstBufSize());
  }
}

void DX12RenderBatch::Draw() {
  Pipe.Set();

  // 设置常量缓冲区的描述符堆
  ID3D12DescriptorHeap* descriptorHeaps[] = {Buffer.DptHeap.Get()};
  Render->Cmd.List->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

  // 寄存器 b1: 物体材质常量
  Render->Cmd.List->SetGraphicsRootConstantBufferView(1,
    Render->MaterialManager.Buffer.DefaultBuffer->GetGPUVirtualAddress());
  // 寄存器 b2: 全局常量
  Render->Cmd.List->SetGraphicsRootDescriptorTable(2,
    Buffer.GetGPUDescriptor(Buffer.GetGlobalConstIndex()));

  // 依次绘制各个物体
  for (auto& O : ROs) {
    Render->MeshManager.Bind();
    // 寄存器 b0: 物体位置常量
    Render->Cmd.List->SetGraphicsRootDescriptorTable(0, Buffer.GetGPUDescriptor(O.ConstBufIndex));
    Render->Cmd.List->DrawIndexedInstanced(O.SubMesh->IndexCount,
    1, O.SubMesh->StartIndexLocation,
    O.SubMesh->BaseVertexLocation, 0);
  }

}


