#include "Core/Const.hlsl"

struct VertexIn
{
	float3 LocalPositon     : POSITION;
    float3 LocalNormal      : NORMAL;
    float2 TexC             : TEXCOORD;
};

struct VertexOut
{
	float4 SVPosition  : SV_POSITION;
    float4 Color : COLOR;
    float2 TexC  : TEXCOORD;
};

// 将屏幕坐标转换为NDC坐标
float4 ScreenToNDC(float2 screenPos, float2 screenSize)
{
    // 将 [0, 1] 范围映射到 [-1, 1] 范围
    float2 ndc = (screenPos / screenSize) * 2.0f - 1.0f;
    // 翻转Y轴
    ndc.y = -ndc.y;
    return float4(ndc, 0.0f, 1.0f);
}

// 计算在世界矩阵下，各个顶点和法线的坐标
VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    float4 posW = mul(float4(vin.LocalPositon, 1.0f), World);
    float2 ScreenPosition = posW.xy;
    vout.SVPosition = ScreenToNDC(ScreenPosition, ScreenSize);
    vout.Color = ObjectColor;
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}

