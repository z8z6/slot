//
// Created by zhou_zhengming on 2026/5/13.
//
#pragma once
#include <DirectXMath.h>

namespace z8
{
struct Transform
{
  // 直角坐标系
  DirectX::XMFLOAT3 Position;  // XYZ 世界位置
  DirectX::XMFLOAT3 Rotation;  // 欧拉角（Pitch/Yaw/Roll）
  DirectX::XMFLOAT3 Scale;     // 缩放
  DirectX::XMFLOAT4X4 World;
  DirectX::XMFLOAT4X4 WorldViewProj;

  // 球坐标系
  // 只用来表示位置
  float Radius;       // 半径（到原点距离）
  float Theta;        // 极角（绕 Y 轴旋转，方位角/水平角）
  float Phi;          // 方位角（从 Y 轴向下量，俯仰角/极角）

  Transform();

  // 从球坐标更新直角坐标
  void UpdateCartesian();

  // 从直角坐标更新球坐标
  void UpdateSpherical();
  void UpdateWorld();
  void UpdateWorldViewProj(const DirectX::XMFLOAT4X4& mProj);

};
}
