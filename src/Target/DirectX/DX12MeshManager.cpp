//
// Created by zhou_zhengming on 2026/5/18.
//

#include "Target/DirectX/DX12MeshManager.h"

#include <d3dcompiler.h>

#include "Target/DirectX/DX12Render.h"
#include "UI/Mesh/Mesh.h"
#include "UI/Object/GameObject.h"

using namespace z8;

DX12MeshManager::DX12MeshManager(DX12Render* R) : DX12Common(R), VBufUpload(R), IBufUpload(R)
{
}

void DX12MeshManager::UnifyMesh()
{
  Mesh.V = Render->GetObjects()->Mesh->V;
  Mesh.I = Render->GetObjects()->Mesh->I;
}

void DX12MeshManager::Init()
{
  UnifyMesh();

  Ok(D3DCreateBlob(Mesh.VSize(), &VBufCPU));
  CopyMemory(VBufCPU->GetBufferPointer(),Mesh.V.data(), Mesh.VSize());

  Ok(D3DCreateBlob(Mesh.ISize(), &IBufCPU));
  CopyMemory(IBufCPU->GetBufferPointer(), Mesh.I.data(), Mesh.ISize());

  VBufGPU = VBufUpload.Upload(Mesh.V.data(), Mesh.VSize());
  IBufGPU = IBufUpload.Upload(Mesh.I.data(), Mesh.ISize());

  Vv.BufferLocation = VBufGPU->GetGPUVirtualAddress();
  Vv.StrideInBytes = Mesh.VElemSize();
  Vv.SizeInBytes = Mesh.VSize();

  Iv.BufferLocation = IBufGPU->GetGPUVirtualAddress();
  Iv.Format = FormatIBuf;
  Iv.SizeInBytes = Mesh.ISize();
}

void DX12MeshManager::Bind() const
{
  Render->Cmd.List->IASetVertexBuffers(0, 1, &Vv);
  Render->Cmd.List->IASetIndexBuffer(&Iv);
  Render->Cmd.List->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
