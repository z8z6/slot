//
// Created by zhou_zhengming on 2026/7/29.
//

#include "Target/DirectX/DX12RenderBatch.h"
#include "Core/Application.h"
#include "Resource/ResourceManager.h"
#include "Target/DirectX/DX12Render.h"

#include <algorithm>
#include <stdexcept>

using namespace z8;

DX12RenderObject::DX12RenderObject(GameObject *O)
: Object(O), SubMesh(nullptr), Pipeline(nullptr), ConstBufIndex(0) {

}

DX12RenderBatch::DX12RenderBatch(DX12Render* render, bool preserveOrder)
    : DX12Common(render), Buffer(this), PreserveOrder(preserveOrder) {}

void DX12RenderBatch::Init(const std::vector<GameObject *> &Os) {
  // 1. 初始化 RO
  // 批次允许为空，UI 声明在运行时发生变化时可安全重建。
  ROs.clear();
  Pipelines.clear();
  unsigned i = 0;
  for (auto* O : Os) {
    DX12RenderObject RO(O);
    RO.Mesh = Render->App->Resources.Resolve(O->Renderable.Mesh);
    RO.Material = Render->App->Resources.Resolve(O->Renderable.Material);
    const auto* material = Render->App->Resources.TryGet(RO.Material);
    if (material) {
      RO.Program = Render->App->Resources.Resolve(material->Program);
      RO.Texture =
          Render->App->Resources.Resolve(material->BaseColorTexture);
      if (!material->BaseColorTexture.GetId().empty() && !RO.Texture.IsValid())
        throw std::runtime_error("Material references an unknown texture.");
    }
    if (!RO.Mesh.IsValid() || !RO.Material.IsValid() || !RO.Program.IsValid())
      throw std::runtime_error("Renderable references an unknown resource.");

    RO.ConstBufIndex = i;
    RO.SubMesh = Render->MeshManager.GetSubMesh(RO.Mesh);
    if (!RO.SubMesh)
      throw std::runtime_error("Renderable mesh has no DX12 submesh.");

    auto pipeline = Pipelines.find(RO.Program);
    if (pipeline == Pipelines.end()) {
      const auto* program = Render->App->Resources.TryGet(RO.Program);
      auto created = std::make_unique<DX12PipelineState>(Render);
      created->Init(*program);
      pipeline = Pipelines.emplace(RO.Program, std::move(created)).first;
    }
    RO.Pipeline = pipeline->second.get();
    ROs.push_back(RO);
    ++i;
  }

  // 相邻 Draw 尽量复用 PSO 和材质绑定；稳定排序保留相同状态内的声明顺序。
  if (!PreserveOrder)
    std::stable_sort(ROs.begin(), ROs.end(),
                     [](const auto& left, const auto& right) {
                       if (left.Program.Index != right.Program.Index)
                         return left.Program.Index < right.Program.Index;
                       if (left.Material.Index != right.Material.Index)
                         return left.Material.Index < right.Material.Index;
                       return left.Mesh.Index < right.Mesh.Index;
                     });

  // 2. 初始化常量缓冲区
  Buffer.InitBuffer();
}

void DX12RenderBatch::Update() const {
  for (auto& O : ROs) {
    O.Object->Update(Render->GetTimer());
    memcpy(Buffer.GetCPUOffset(O.ConstBufIndex), O.Object->ConstBuf(), O.Object->ConstBufSize());
  }
}

void DX12RenderBatch::Draw() {
  if (ROs.empty()) return;
  // b0/b1/b2 使用根 CBV，因此唯一 Shader-visible 描述符堆可稳定留给纹理 SRV。
  Render->TextureManager.Bind();

  // 寄存器 b2: 全局常量
  Render->Cmd.List->SetGraphicsRootConstantBufferView(
      2, Buffer.GetGPUAddress(Buffer.GetGlobalConstIndex()));

  // 依次绘制各个物体
  for (auto& O : ROs) {
    O.Pipeline->Set();
    // 寄存器 b1: 每个 RenderItem 绑定自己的 256 字节对齐材质常量。
    Render->Cmd.List->SetGraphicsRootConstantBufferView(
        1, Render->MaterialManager.GetGPUAddress(O.Material));
    if (O.Texture.IsValid())
      Render->Cmd.List->SetGraphicsRootDescriptorTable(
          3, Render->TextureManager.GetGPUDescriptor(O.Texture));
    Render->MeshManager.Bind();
    // 寄存器 b0: 物体位置常量
    Render->Cmd.List->SetGraphicsRootConstantBufferView(
        0, Buffer.GetGPUAddress(O.ConstBufIndex));
    Render->Cmd.List->DrawIndexedInstanced(O.SubMesh->IndexCount,
    1, O.SubMesh->StartIndexLocation,
    O.SubMesh->BaseVertexLocation, 0);
  }

}

