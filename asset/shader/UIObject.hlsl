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
    // Layout 始终保存主工作区坐标；不同 HWND 只通过 UIOrigin 建立本地视口，
    // 不复制或临时改写控件常量，因此 Dock/Floating 仍共享同一棵控件树。
    // Layout 保持 96 DPI 逻辑坐标，最终顶点才映射到交换链物理像素；这样
    // 高 DPI 下控件尺寸稳定，同时避免 DWM 对整帧做模糊的位图放大。
    vout.SVPosition =
        ScreenToNDC((ScreenPosition - UIOrigin) * UIScale, ScreenSize);
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
    float2 size = max(RectBounds.zw - RectBounds.xy, 0.0f);
    float2 center = (RectBounds.xy + RectBounds.zw) * 0.5f;
    float radius = min(max(CornerRadius, 0.0f), min(size.x, size.y) * 0.5f);
    // 圆角矩形 SDF 在屏幕像素空间计算：dist=0 是轮廓，负值在内部。
    // 这样圆角与边框不会随矩形缩放改变视觉粗细，并能共用同一裁剪路径。
    float2 rounded = abs(pin.ScreenPosition - center) - size * 0.5f + radius;
    float distanceToShape = length(max(rounded, 0.0f)) +
                            min(max(rounded.x, rounded.y), 0.0f) - radius;
    if (distanceToShape > 0.0f)
        discard;

    if (VisualType > 0.5f) {
        float2 uv = (pin.ScreenPosition - RectBounds.xy) / max(size, 1.0f);
        // Lucide SVG 自身已经包含 viewBox 安全边距；若再直接使用完整 [-1,1]
        // 坐标，轮廓只占 ImageNode 中央约六成。缩小局部坐标等价于放大图标，
        // 同时保留约一个像素的 MSAA 抗锯齿覆盖区，避免笔画贴边被裁掉。
        float2 p = (uv * 2.0f - 1.0f) * 0.76f;
        float stroke = 0.12f;
        float iconDistance = 1.0f;
        if (ImageKind < 1.5f) {
            // Close：两条有限对角线，端点夹紧避免笔画伸出图标框。
            float2 a = abs(p);
            iconDistance = min(abs(p.x - p.y), abs(p.x + p.y));
            if (max(a.x, a.y) > 0.62f)
                iconDistance = 1.0f;
        } else if (ImageKind < 2.5f) {
            // Plus：水平与垂直短线组合。
            iconDistance = min(max(abs(p.x) - 0.58f, abs(p.y)),
                               max(abs(p.y) - 0.58f, abs(p.x)));
        } else if (ImageKind < 3.5f) {
            // ChevronDown：两段向下折线。
            iconDistance = abs(abs(p.x) + p.y - 0.28f);
            if (abs(p.x) > 0.62f || p.y < -0.42f || p.y > 0.45f)
                iconDistance = 1.0f;
        } else if (ImageKind < 4.5f) {
            // Cube：菱形顶面与三条结构边，适合小尺寸 Panel 图标。
            float diamond = abs(abs(p.x) + abs(p.y + 0.18f) - 0.62f);
            float vertical = abs(p.x) + (p.y < -0.18f || p.y > 0.65f ? 1.0f : 0.0f);
            iconDistance = min(diamond, vertical);
        } else if (ImageKind < 5.5f) {
            // Terminal：外框内叠加提示符和下划线，轮廓取自 Lucide terminal。
            float frame = abs(max(abs(p.x), abs(p.y)) - 0.72f);
            float prompt = abs(abs(p.x + 0.28f) + p.y - 0.16f);
            if (p.x < -0.58f || p.x > 0.04f || abs(p.y) > 0.38f)
                prompt = 1.0f;
            float underline = max(abs(p.y - 0.30f), abs(p.x - 0.30f) - 0.22f);
            iconDistance = min(frame, min(prompt, underline));
        } else if (ImageKind < 6.5f) {
            // Settings-2：两条滑杆和错开的圆形旋钮与 Lucide 源资源保持同一
            // 语义轮廓；使用 SDF 组合后仍能随 Theme Tint 单色绘制。
            float topLine = max(abs(p.y + 0.42f), abs(p.x - 0.18f) - 0.52f);
            float bottomLine = max(abs(p.y - 0.42f), abs(p.x + 0.18f) - 0.52f);
            float topKnob = abs(length(p - float2(-0.42f, -0.42f)) - 0.18f);
            float bottomKnob = abs(length(p - float2(0.42f, 0.42f)) - 0.18f);
            iconDistance = min(min(topLine, bottomLine),
                               min(topKnob, bottomKnob));
        } else {
            // ChevronRight：与展开态 ChevronDown 使用相同折线尺度，旋转语义
            // 由独立图标表达，避免 TreeItem 为一个状态引入矩阵常量。
            iconDistance = abs(abs(p.y) - p.x - 0.28f);
            if (abs(p.y) > 0.62f || p.x < -0.42f || p.x > 0.45f)
                iconDistance = 1.0f;
        }
        if (iconDistance > stroke)
            discard;
        return ObjectColor;
    }

    // SDF 的相反数是到外轮廓的内部距离，可直接得到固定像素边框。
    float edgeDistance = -distanceToShape;
    if (BorderWidth > 0.0f && edgeDistance <= BorderWidth)
        return BorderColor;
    return pin.Color;
}

