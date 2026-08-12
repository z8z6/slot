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
    float2 ScreenPosition : CLIPPOSITION;
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
    vout.ScreenPosition = ScreenPosition;
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    // 视口裁剪在像素空间执行，滚动子项无需拆分几何或逐控件切换 scissor。
    if (pin.ScreenPosition.x < ClipRect.x || pin.ScreenPosition.y < ClipRect.y ||
        pin.ScreenPosition.x > ClipRect.z || pin.ScreenPosition.y > ClipRect.w)
        discard;
    // 边框宽度使用屏幕像素而非局部坐标，因此控件缩放后仍保持一致的视觉重量。
    float edgeDistance = min(min(pin.ScreenPosition.x - RectBounds.x,
                                 RectBounds.z - pin.ScreenPosition.x),
                             min(pin.ScreenPosition.y - RectBounds.y,
                                 RectBounds.w - pin.ScreenPosition.y));
    if (BorderWidth > 0.0f && edgeDistance <= BorderWidth)
        return BorderColor;
    return pin.Color;
}

