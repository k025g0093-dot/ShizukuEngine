#include "Object3d.hlsli"

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

StructuredBuffer<InstanceData> gInstances : register(t2);
struct DrawInfo
{
      uint baseInstance;
};
ConstantBuffer<DrawInfo> gDrawInfo : register(b1);

static const int MAX_BONES = 100;
static const int MAX_BONE_INFLUENCE = 4;

struct BoneMatrices
{
      row_major float4x4 finalBonesMatrices[MAX_BONES];
};

ConstantBuffer<BoneMatrices> gBoneMatrices : register(b5);


VertexShaderOutput main(SkinnedVertexShaderInput input, uint instanceId : SV_InstanceID)
{
      VertexShaderOutput output;
      InstanceData instance = gInstances[gDrawInfo.baseInstance + instanceId];
      
      float3 totalNormal = float3(0.0f, 0.0f, 0.0f);
      float4 totalPosition = float4(0.0f, 0.0f, 0.0f, 0.0f);
      for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
      {
            
            if (input.boneIds[i] == -1)
                  continue;
            
            if (input.boneIds[i] >= MAX_BONES)
            {
                  totalPosition = float4(input.position, 1.0f);
            }
            float4 localPosition = mul(float4(input.position, 1.0f), gBoneMatrices.finalBonesMatrices[input.boneIds[i]]);
            totalPosition += localPosition * input.weights[i];
            float3 localNormal = mul(input.normal, (float3x3) gBoneMatrices.finalBonesMatrices[input.boneIds[i]]);
            
            totalNormal += localNormal * input.weights[i];
      }
      
      output.position = mul(totalPosition, instance.WVP);
      output.texcoord = input.texCoords;
      output.normal = normalize(mul(totalNormal, (float3x3) instance.World));
      output.tangent = normalize(mul(input.tangent, (float3x3) instance.World));
      output.worldPosition = mul(totalPosition, instance.World).xyz;
      
      return output;

}
