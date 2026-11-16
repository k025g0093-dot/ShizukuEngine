#define float32_t4 float4



struct VertexShaderOutput
{
      float4 position : SV_POSITION;
      float2 texcoord : TEXCOORD;
      float3 normal : NORMAL0;
      float3 tangent : TANGENT0;
      float3 worldPosition : POSITION0;
      float4 color : COLOR0;
};

struct InstanceData
{
      row_major float4x4 WVP;
      row_major float4x4 World;
      float4 Color;
};

struct Particle
{
    float3 translate;
    float3 scale;
    float3 padding;
    float3 velocity;
    float4 color;
    float lifeTime;
    float currentTime;
};

struct PerView
{
    row_major float4x4 viewProjection;
    row_major float4x4  billboardMatrix;
};
