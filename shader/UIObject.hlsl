#include "Core/Const.hlsl"

struct VertexIn
{
	float3 LocalPositon     : POSITION;
    float3 LocalNormal      : NORMAL;
    float2 TexC             : TEXCOORD;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
    float2 TexC  : TEXCOORD;
};

// 计算在世界矩阵下，各个顶点和法线的坐标
VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    vout.PosH = mul(float4(vin.LocalPositon, 1.0f), World);
    vout.Color = float4(1.0f, 0.0f, 1.0f, 1.0f);
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}

