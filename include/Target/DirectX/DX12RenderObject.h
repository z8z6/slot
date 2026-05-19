//
// Created by zhou_zhengming on 2026/5/19.
//

#pragma once

namespace z8 {
class GameObject;
class DX12SubMesh;
class DX12RenderObject {
public:
  GameObject* Object;
  DX12SubMesh* SubMesh;
  unsigned ConstBufIndex;

  DX12RenderObject(GameObject* O);
};
} // namespace z8
