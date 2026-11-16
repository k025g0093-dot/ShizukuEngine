#include "Animator.h"
#include "../SkinningConstants.h"
#include "AssimpGLMHelpersMath/GlmConvert.h"

void Animator::Initialize(Animation* animation) {

	mCurrentTime = 0.0f;
	mCurrentAnimation = animation;

	mFinalBoneMatrices.reserve(kMaxBones);
	for (int i = 0; i < kMaxBones; i++) {
		matrix.assign(kMaxBones, MakeIdentity4x4());
		mFinalBoneMatrices.push_back(glm::mat4(1.0f));
	}
}

void Animator::UpdateAnimation(float dt)
{
	mDeltaTime = dt;
	if (mCurrentAnimation)
	{
		mCurrentTime += mCurrentAnimation->GetTicksPerSecond() * dt;
		mCurrentTime = fmod(mCurrentTime, mCurrentAnimation->GetDuration());
		CalculateBoneTransform(&mCurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		for (int i = 0; i < kMaxBones; i++) {
			matrix[i] = GlmConvert::ConvertGLMtoMatrix(mFinalBoneMatrices[i]);
		}
	}
}

void Animator::PlayAnimation(Animation* pAnimation)
{
	mCurrentAnimation = pAnimation;
	mCurrentTime = 0.0f;
}

void Animator::CalculateBoneTransform(
	const AssimpNodeData* node,
	glm::mat4 parentTransform
) {

	std::string nodeName = node->name;
	glm::mat4 nodeTransform = node->transformation;

	const Bone* bone = mCurrentAnimation->FindBone(nodeName);

	if (bone) {
		nodeTransform = bone->CalculateLocalTransform(mCurrentTime);
	}

	glm::mat4 globalTransformation = parentTransform * nodeTransform;

	auto boneInFoMap = mCurrentAnimation->GetBoneIDMap();
	if (boneInFoMap.find(nodeName) != boneInFoMap.end()) {
		int index = boneInFoMap[nodeName].id;
		glm::mat4 offset = boneInFoMap[nodeName].offset;
		mFinalBoneMatrices[index] = globalTransformation * offset;
	}
	for (int i = 0; i < node->childrenCount; i++) {
		CalculateBoneTransform(&node->children[i], globalTransformation);
	}
}


