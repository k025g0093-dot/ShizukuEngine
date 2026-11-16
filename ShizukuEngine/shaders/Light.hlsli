struct Light
{
    float3 direction;
    float3 position;
    float4 color;
    int type;
    float intensity;
    float distance;
    float decay;
    float cosAngle;
    float cosFalloffStart;
};

StructuredBuffer<Light> g_lights : register(t3);

cbuffer LightCountBuffer : register(b3)
{
    uint gActiveLightCount;
};

cbuffer ShadowLightBuffer : register(b4)
{
    row_major float4x4 gLightVP;
};
