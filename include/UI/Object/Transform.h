//
// Created by zhou_zhengming on 2026/5/13.
//
#pragma once
#include <DirectXMath.h>
#include <iosfwd>

/**
 * 描述坐标系，左手坐标系
 * 使用 DirectX12 坐标系的描述
 * 1. +X: 右，+Y: 上， +Z: 屏幕里
 * 2. 由于UI相对于地面xz是立起来的，所以UI是xy平面
 *
 */
namespace z8
{
struct Transform
{
  // 直角坐标系描述
  DirectX::XMFLOAT3 Position;  // XYZ 世界位置

  // 球坐标系描述
  // 只用来表示位置
  float Radius;       // 半径，到原点距离
  float Theta;        // 与x轴夹角，水平面
  float Phi;          // 与y轴夹角，竖直面

  DirectX::XMFLOAT3 Rotation;  // 欧拉角（Pitch/Yaw/Roll）
  DirectX::XMFLOAT3 Scale;     // 缩放

  DirectX::XMFLOAT4X4 World;
  DirectX::XMFLOAT4X4 WorldViewProj;

  Transform();

  // 从球坐标更新直角坐标
  void UpdateCartesian();

  // 从直角坐标更新球坐标
  void UpdateSpherical();
  void UpdateWorld();
  void UpdateWorldViewProj(
    const DirectX::XMFLOAT4X4& View,
    const DirectX::XMFLOAT4X4& Proj);

};

std::ostream& operator<<(std::ostream& o, const Transform& transform);
}
