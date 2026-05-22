#include "Light.hlsl"

cbuffer cbPerObject : register(b0)
{
	float4x4 World;
};

cbuffer cbPass : register(b1)
{
    float TimeCost;
    float TimeTotal;
    float4x4 ViewProj;
};

struct VertexIn
{
	float3 LocalPositon     : POSITION;
    float3 LocalNormal      : NORMAL;
};

struct VertexOut
{
	float4 SVPositon        : SV_POSITION;
    float3 WorldPositon     : POSITION;
    float3 WorldNormal      : NORMAL;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

    float4 posW = mul(float4(vin.LocalPositon, 1.0f), World);
    vout.WorldPositon = posW.xyz;
    vout.SVPositon = mul(posW, ViewProj);
    // @todo 非归一化变化需要转置逆矩阵
    float3x3 normalMatrix = (float3x3)World;
    vout.WorldNormal = mul(vin.LocalNormal, normalMatrix);
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return float4(0.2588f, 0.8f, 1.0f, 1.0f);
}


