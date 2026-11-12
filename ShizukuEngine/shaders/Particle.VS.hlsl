#include "Particle.hlsli"

StructuredBuffer<InstanceData> gInstances : register(t2);

struct DrawInfo
{
    uint baseInstance;
};

ConstantBuffer<DrawInfo> gDrawInfo : register(b1);

struct VertexShaderInput
{
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL0;
    float3 tangent : TANGENT0;
};

StructuredBuffer<Particle> gParticle : register(t0);
ConstantBuffer<PerView> gPerView : register(b0);

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    InstanceData instance = gInstances[gDrawInfo.baseInstance + instanceId];
      
    Particle particle = gParticle[instanceId];
    VertexShaderOutput output;
    
    float4x4 worldMatrix = gPerView.billboardMatrix; //wolrdMatrixの作成
    
    worldMatrix[0] *= particle.scale.x;
    worldMatrix[1] *= particle.scale.y;
    worldMatrix[2] *= particle.scale.z;
    worldMatrix[3].xyz *= particle.translate;
    
    output.position = mul(input.position, mul(worldMatrix, gPerView.viewProjection));
    output.texcoord = input.texcoord;
    output.color = instance.Color;
    return output;
}