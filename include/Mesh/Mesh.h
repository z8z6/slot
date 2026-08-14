//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once

#include "Vertex.h"

#include <string>
#include <vector>

/**
 * 描述物体顶点
 * 1. 从相机视角，顶点绕序决定正反面，顺时针为正面，逆时针为背面
 */

namespace z8 {
/** 指定资源注册时是否应重新生成法线。 */
enum class MeshNormalMode {
  GenerateSmooth,
  PreserveAuthored,
};

class Mesh {
public:
  using IndexTy = uint16_t;
  std::vector<Vertex> V;
  std::vector<IndexTy> I;
  std::string Name;
  // FBX 的分裂法线及球体解析法线不能再做跨顶点平均，否则会破坏硬边或精确曲率。
  MeshNormalMode NormalMode = MeshNormalMode::GenerateSmooth;

  Mesh() = default;
  virtual ~Mesh() = default;

  unsigned VSize() const { return V.size() * sizeof(Vertex); }
  unsigned VElemSize() const { return sizeof(Vertex); }
  unsigned ISize() const { return I.size() * sizeof(IndexTy); }
  unsigned ICount() const { return I.size(); }
  /** 按三角形面积加权生成平滑法线，并安全跳过非法或退化三角形。 */
  void ComputeNormals();
  /** 验证 16 位索引网格可安全上传；失败时可返回英文诊断信息。 */
  bool Validate(std::string* error = nullptr) const;
};
} // namespace z8
