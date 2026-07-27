#include "Core/Const.hlsl"

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


// 计算在世界矩阵下，各个顶点和法线的坐标
VertexOut VS(VertexIn vin)
{
	VertexOut vout;

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
    pin.WorldNormal = normalize(pin.WorldNormal);
    float3 toEyeW = normalize(Camera - pin.WorldPositon);
    float4 ggAlbedo = float4(0.133333340f, 0.545098066f, 0.133333340f, 1.f);
    float3 ggFresnelR0 = float3(0.02f, 0.02f, 0.02f);
    float ggRough = 0.2f;
    const float shininess = 1.0f - ggRough;
    Material mat = { ggAlbedo, ggFresnelR0, shininess };
    float3 shadowFactor = 1.0f;

    float4 Direct = ComputeLighting(gLight, mat, pin.WorldPositon, pin.WorldNormal, toEyeW, shadowFactor);
    float4 Ambient = AmbientLight * ggAlbedo;
    float4 color = Ambient + Direct;
    return color;
}


