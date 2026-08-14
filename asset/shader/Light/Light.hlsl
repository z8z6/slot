struct Light {
  float3 Position;
  float p0;
  float3 Color;
  float p1;
  float3 Direction;
  float p2;
};

struct Material
{
   float4 Albedo;
   float3 FresnelR0;
   float Roughness;
};


