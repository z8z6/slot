cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
};

cbuffer cbPass : register(b1)
{
    float TimeCost;
    float TimeTotal;
};

struct VertexIn
{
	float3 PosL  : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	float wave = sin(TimeTotal) * 0.5f + 0.5f;
    vout.Color = vin.Color * wave;
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}


