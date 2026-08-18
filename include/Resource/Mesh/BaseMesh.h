//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once

#include "BuiltinResource.h"
#include "Resource/ResourceHandle.h"
#include "Vertex.h"
#include <string>
#include <vector>

namespace z8 {
enum class NormalTy {
  Generate,
  Preserve,
};

/**
 * 描述物体顶点
 * 从相机视角，顶点绕序决定正反面，顺时针为正面，逆时针为背面
 */
class BaseMesh : public Resource{
public:
  using IndexTy = uint16_t;
  std::vector<Vertex> V;
  std::vector<IndexTy> I;

  // 导入 FBX 模型的法线不能再做跨顶点平均，否则会破坏硬边或精确曲率
  NormalTy NormalMode = NormalTy::Generate;

  BaseMesh(){
    Type = ResourceTy::Mesh;
    Id = builtin::mesh::MeshPrefix;
  }

  unsigned VertexByteSize() const { return V.size() * sizeof(Vertex); }
  unsigned VertexElementSize() const { return sizeof(Vertex); }
  unsigned VertexCount() const { return V.size(); }
  unsigned IndexByteSize() const { return I.size() * sizeof(IndexTy); }
  unsigned IndexCount() const { return I.size(); }
  void ComputeNormals();
  bool Validate(std::string* error = nullptr) const;
  virtual void Update(){}
};

struct MeshImporter {
  BaseMesh Parse(std::string FileName);
};
} // namespace z8
