#include "Light/Phong.hlsl"

cbuffer cbPerObject : register(b0)
{
	float4x4 World;
    // 非均匀缩放下法线必须乘世界逆转置矩阵，不能直接复用 World。
    float4x4 WorldInvTranspose;
    float4 ObjectColor;
    float4 ClipRect;
    float4 BorderColor;
    float4 RectBounds;
    float BorderWidth;
    float CornerRadius;
    float VisualType;
    float ImageKind;
};

cbuffer cbMaterial : register(b1)
{
	float4 gAlbedo;
    float3 gFresnelR0;
    float  gRough;
};

cbuffer cbPass : register(b2)
{
    float4x4 ViewProj;
    Light gLight;
    float4 AmbientLight;
    float3 Camera;
    float p5;
    float2 ScreenSize;
    float2 UIOrigin;
    float TimeCost;
    float TimeTotal;
};

