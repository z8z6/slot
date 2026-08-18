//
// Created by zhou_zhengming on 2026/7/29.
//

#pragma once
#include "DX12Common.h"
#include "DX12ConstBuffer.h"
#include "DX12PipelineState.h"
#include "Material/BaseMaterial.h"
#include "Mesh/BaseMesh.h"
#include "Resource/ResourceHandle.h"
#include "Shader/BaseShaderProgram.h"
#include "Texture/BaseTexture.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace z8 {
class GameObject;
struct DX12SubMesh;

/**
 * @brief 描述渲染物体所需的数据
 * 1. 网格索引
 * 2. 常量缓冲区索引
 * 该类是值拷贝
 */
class DX12RenderObject {
public:
  GameObject* Object;
  DX12SubMesh* SubMesh;
  ResourceHandle<BaseMesh> Mesh;
  ResourceHandle<BaseMaterial> Material;
  ResourceHandle<BaseShaderProgram> Program;
  ResourceHandle<BaseTexture> Texture;
  DX12PipelineState* Pipeline;
  unsigned ConstBufIndex;

  explicit DX12RenderObject(GameObject* O);
};

class DX12RenderBatch : public DX12Common {
public:
  std::vector<DX12RenderObject> ROs;
  DX12ConstBuffer Buffer;

  explicit DX12RenderBatch(DX12Render* render, bool preserveOrder = false);

  void Init(const std::vector<GameObject*>& Os);
  void Update() const;
  void Draw();

private:
  std::unordered_map<ResourceHandle<BaseShaderProgram>,
                     std::unique_ptr<DX12PipelineState>,
                     ResourceHandleHash<BaseShaderProgram>> Pipelines;
  // 透明 UI 必须维持画家顺序；不透明 3D 才能按状态排序减少切换。
  bool PreserveOrder;
};
}
