//
// Created by zhou_zhengming on 2026/5/18.
//

#include "Target/DirectX/DX12MeshManager.h"
#include <d3dcompiler.h>
#include "UI/Mesh/MeshRegistry.h"
#include "Target/DirectX/DX12Render.h"
#include "UI/Mesh/Mesh.h"
#include "UI/Object/GameObject.h"

using namespace z8;

DX12MeshManager::DX12MeshManager(DX12Render* R) : DX12Common(R), VBufUpload(R), IBufUpload(R)
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

  Ok(D3DCreateBlob(MergeMesh.VSize(), &VBufCPU));
  CopyMemory(VBufCPU->GetBufferPointer(),MergeMesh.V.data(), MergeMesh.VSize());

  Ok(D3DCreateBlob(MergeMesh.ISize(), &IBufCPU));
  CopyMemory(IBufCPU->GetBufferPointer(), MergeMesh.I.data(), MergeMesh.ISize());

  VBufGPU = VBufUpload.Upload(MergeMesh.V.data(), MergeMesh.VSize());
  IBufGPU = IBufUpload.Upload(MergeMesh.I.data(), MergeMesh.ISize());

  Vv.BufferLocation = VBufGPU->GetGPUVirtualAddress();
  Vv.StrideInBytes = MergeMesh.VElemSize();
  Vv.SizeInBytes = MergeMesh.VSize();

  Iv.BufferLocation = IBufGPU->GetGPUVirtualAddress();
  Iv.Format = FormatIBuf;
  Iv.SizeInBytes = MergeMesh.ISize();
}

void DX12MeshManager::Bind() const
{
  Render->Cmd.List->IASetVertexBuffers(0, 1, &Vv);
  Render->Cmd.List->IASetIndexBuffer(&Iv);
  Render->Cmd.List->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

DX12SubMesh* DX12MeshManager::GetSubMesh(Mesh *Mesh) {
  if (!SubMeshes.count(Mesh)) return nullptr;
  return &SubMeshes[Mesh];
}
