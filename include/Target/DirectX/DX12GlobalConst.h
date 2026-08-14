//
// Created by zhou_zhengming on 2026/5/22.
//

#pragma once
#include <DirectXMath.h>
#include <cstddef>

namespace z8 {
class DX12Render;
class DX12RenderBatch;

struct DX12Light {
  DirectX::XMFLOAT3 Position;
  float p0;
  DirectX::XMFLOAT3 Strength;
  float p1;
  DirectX::XMFLOAT3 Direction;
  float p2;
};

/**
 * @brief CPU 侧全局常量
 * @note 必须注意对齐，GPU侧的类布局可能不一致
 **/
struct  DX12GlobalConst {
  DirectX::XMFLOAT4X4A ViewProj;
  DX12Light Light;
  DirectX::XMFLOAT4 AmbientLight;
  DirectX::XMFLOAT3 Camera;
  float p0;
  DirectX::XMFLOAT2 ScreenSize;
  /** UI 顶点仍保存 Layout 全局坐标；独立宿主用该原点映射到本地 NDC。 */
  DirectX::XMFLOAT2 UIOrigin = {0.0f, 0.0f};
  float TimeCost;
  float TimeTotal;
  /** 96 DPI Layout 坐标到当前交换链物理像素的比例。 */
  float UIScale = 1.0f;
  float Padding = 0.0f;

  void Update(DX12Render* R);
  /** 把当前全局状态写入指定批次，供多个交换链复用同一帧状态。 */
  void WriteToBatch(DX12RenderBatch &batch) const;
  static unsigned AlignedSize();

private:
  void WriteToBuffer(DX12Render* R) const;
};
static_assert(offsetof(DX12GlobalConst, UIScale) == 168,
              "UI scale must match cbPass packing in Const.hlsl.");
static_assert(sizeof(DX12GlobalConst) == 176,
              "Global constants must match cbPass size in Const.hlsl.");


}




