//
// Created by zhou_zhengming on 2026/5/18.
//

#include "Target/DirectX/DX12MeshManager.h"
#include "UI/Mesh/MeshRegistry.h"
#include "Target/DirectX/DX12Render.h"
#include "UI/Mesh/Mesh.h"


using namespace z8;

DX12MeshManager::DX12MeshManager(DX12Render* R) : DX12Common(R), VBuf(R), IBuf(R)
{
}

void DX12MeshManager::UnifyMesh()
{
  SubMeshes.clear();
  for (auto* M : MeshRegistry::Instance().Meshes) {
    DX12SubMesh SubMesh;
    SubMesh.IndexCount = M->I.size();
    SubMesh.BaseVertexLocation = MergeMesh.V.size();
    SubMesh.StartIndexLocation = MergeMesh.I.size();

    MergeMesh.V.insert(MergeMesh.V.end(), M->V.begin(), M->V.end());
    MergeMesh.I.insert(MergeMesh.I.end(), M->I.begin(), M->I.end());

    SubMeshes[M] = SubMesh;
  }
}

void DX12MeshManager::Init()
{
  UnifyMesh();

  VBuf.Init(MergeMesh.VSize());
  VBuf.Update(MergeMesh.V.data());
  IBuf.Init(MergeMesh.ISize());
  IBuf.Update(MergeMesh.I.data());


  Vv.BufferLocation = VBuf.DefaultBuffer->GetGPUVirtualAddress();
  Vv.StrideInBytes = MergeMesh.VElemSize();
  Vv.SizeInBytes = MergeMesh.VSize();

  Iv.BufferLocation = IBuf.DefaultBuffer->GetGPUVirtualAddress();
  Iv.Format = FormatIBuf;
  Iv.SizeInBytes = MergeMesh.ISize();
}

void DX12MeshManager::Bind() const
{
  // 指定顶点缓冲区
  Render->Cmd.List->IASetVertexBuffers(0, 1, &Vv);
  // 指定顶点索引缓冲区
  Render->Cmd.List->IASetIndexBuffer(&Iv);
  // 指定顶点组合方式为三角形列表
  Render->Cmd.List->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

DX12SubMesh* DX12MeshManager::GetSubMesh(Mesh *Mesh) {
  if (!SubMeshes.count(Mesh)) return nullptr;
  return &SubMeshes[Mesh];
}
