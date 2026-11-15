#include "Object3d.hlsli"

StructuredBuffer<InstanceData> gInstances : register(t2);

cbuffer DrawInfoBuffer : register(b1)
{
    uint baseInstance;
};

cbuffer LightVPBuffer : register(b4)
{
    row_major float4x4 gLightVP;
};

struct SkinnedVertexShaderInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoords : TEXCOORD;
    float3 tangent : TANGENT;
    float3 biTangent : BINORMAL;
    int4 boneIds : BLENDINDICES;
    float4 weights : BLENDWEIGHT;
};

static const int MAX_BONES = 200;
static const int MAX_BONE_INFLUENCE = 4;

struct BoneMatrices
{
    row_major float4x4 finalBonesMatrices[MAX_BONES];
};

ConstantBuffer<BoneMatrices> gBoneMatrices : register(b5);

float4 main(SkinnedVertexShaderInput input, uint instanceId : SV_InstanceID) : SV_POSITION
{
    InstanceData instance = gInstances[baseInstance + instanceId];
    float4 totalPosition = float4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
            
        if (input.boneIds[i] == -1)
            continue;
        if (input.boneIds[i] >= MAX_BONES){continue;}
        
        float4 localPosition = mul(float4(input.position, 1.0f), gBoneMatrices.finalBonesMatrices[input.boneIds[i]]);
        totalPosition += localPosition * input.weights[i];
    }
    
    float4 pos = totalPosition;
    pos = mul(pos, instance.World);
    pos = mul(pos, gLightVP);
    return pos;
}