#define float32_t4 float4
#define float32_t4x4 float4x4
#include "Particle.hlsli"

struct Material
{
    float32_t4 color;
     float4 padding;
    float32_t4x4 uvTransform;
};

ConstantBuffer<Material> gMaterial : register(b0);

struct Camera
{
      float3 worldPosition;
};

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

Texture2D<float32_t4> gTexture : register(t0);
Texture2D<float32_t4> gNormalTexture : register(t1);
SamplerState gSampler : register(s0);
ConstantBuffer<Camera> gCamera : register(b2);

PixelShaderOutput main(VertexShaderOutput input)
{
      PixelShaderOutput output;

      float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);

    float32_t4 textureColor;

      textureColor = gTexture.Sample(gSampler, transformedUV.xy);
      output.color = input.color * textureColor;

      if (textureColor.a == 0.0f)
      {
            discard;
      }
      
      return output;
}