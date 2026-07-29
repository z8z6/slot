//
// Created by zhou_zhengming on 2026/7/29.
//

#pragma once
#include "DX12Common.h"
#include "DX12ConstBuffer.h"
#include "DX12PipelineState.h"

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
  unsigned ConstBufIndex;

  explicit DX12RenderObject(GameObject* O);
};

class DX12RenderBatch : public DX12Common {
public:
  std::vector<DX12RenderObject> ROs;
  DX12ConstBuffer Buffer;
  DX12PipelineState Pipe;

  explicit DX12RenderBatch(DX12Render* R);

  void Init(std::vector<GameObject*>& Os);
  void Update() const;
  void Draw();
};
}
