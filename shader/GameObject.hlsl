#include "Core/Const.hlsl"

struct VertexIn
{
	float3 LocalPositon     : POSITION;
    float3 LocalNormal      : NORMAL;
    float2 TexC             : TEXCOORD;
};

struct VertexOut
{
	float4 SVPositon        : SV_POSITION;
    float3 WorldPositon     : POSITION;
    float3 WorldNormal      : NORMAL;
    float2 TexC             : TEXCOORD;
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
    vout.TexC = vin.TexC;
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    pin.WorldNormal = normalize(pin.WorldNormal);
    float3 toEyeW = normalize(Camera - pin.WorldPositon);
    // 材质常量由 RenderItem 在 b1 逐对象绑定；共享 Mesh 不再隐式使用固定 Metal 数据。
    const float shininess = 1.0f - gRough;
    Material mat = { gAlbedo, gFresnelR0, shininess };
    float3 shadowFactor = 1.0f;

    float4 Direct = ComputeLighting(gLight, mat, pin.WorldPositon, pin.WorldNormal, toEyeW, shadowFactor);
    float4 Ambient = AmbientLight * gAlbedo;
    float4 color = Ambient + Direct;
    return color;
}


