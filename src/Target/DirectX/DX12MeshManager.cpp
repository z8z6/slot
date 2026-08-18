//
// Created by zhou_zhengming on 2026/5/18.
//

#include "Target/DirectX/DX12MeshManager.h"
#include "Core/Application.h"
#include "Mesh/BaseMesh.h"
#include "Resource/ResourceManager.h"
#include "Target/DirectX/DX12Render.h"

using namespace z8;

DX12MeshManager::DX12MeshManager(DX12Render* R) : DX12Common(R), VBuf(R), IBuf(R)
{
}

void DX12MeshManager::UnifyMesh()
{
  SubMeshes.clear();
  // 将注册过的所有 Mesh 拼成一个 Mesh
  // 同一种 Mesh 只会出现一次
  Render->App->Resources.Meshes.Visit(
      [this](ResourceHandle<BaseMesh> handle, const BaseMesh& mesh) {
    DX12SubMesh SubMesh;
    SubMesh.IndexCount = mesh.I.size();
    SubMesh.BaseVertexLocation = MergeMesh.V.size();
    SubMesh.StartIndexLocation = MergeMesh.I.size();

    MergeMesh.V.insert(MergeMesh.V.end(), mesh.V.begin(), mesh.V.end());
    MergeMesh.I.insert(MergeMesh.I.end(), mesh.I.begin(), mesh.I.end());

    SubMeshes[handle] = SubMesh;
  });
}

void DX12MeshManager::Init()
{
  UnifyMesh();

  VBuf.Init(MergeMesh.VertexByteSize());
  VBuf.Update(MergeMesh.V.data());
  IBuf.Init(MergeMesh.IndexByteSize());
  IBuf.Update(MergeMesh.I.data());


  Vv.BufferLocation = VBuf.DefaultBuffer->GetGPUVirtualAddress();
  Vv.StrideInBytes = MergeMesh.VertexElementSize();
  Vv.SizeInBytes = MergeMesh.VertexByteSize();

  Iv.BufferLocation = IBuf.DefaultBuffer->GetGPUVirtualAddress();
  Iv.Format = FormatIBuf;
  Iv.SizeInBytes = MergeMesh.IndexByteSize();
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

DX12SubMesh* DX12MeshManager::GetSubMesh(ResourceHandle<BaseMesh> mesh) {
  const auto iterator = SubMeshes.find(mesh);
  return iterator == SubMeshes.end() ? nullptr : &iterator->second;
}
