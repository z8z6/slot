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
    uint gHasBaseColorTexture;
    float3 gMaterialPadding;
};

cbuffer cbPass : register(b2)
{
    float4x4 ViewProj;
    // 固定上限保证 cbPass ABI 可预测；CPU 每帧只上传有效前缀及数量。
    Light gLights[8];
    float4 AmbientLight;
    float3 Camera;
    uint LightCount;
    float2 ScreenSize;
    float2 UIOrigin;
    float TimeCost;
    float TimeTotal;
    float UIScale;
    float p6;
};

