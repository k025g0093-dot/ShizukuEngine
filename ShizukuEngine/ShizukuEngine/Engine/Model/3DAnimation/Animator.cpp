#include "Animator.h"
#include "../SkinningConstants.h"
#include "AssimpGLMHelpersMath/GlmConvert.h"

void Animator::Initialize(Animation* animation) {

	mCurrentTime = 0.0f;
	mCurrentAnimation = animation;
	mGlobalBoneMatrices.assign(kMaxBones, glm::mat4(1.0f));

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
		if (mIsLooped) {
			mCurrentTime = fmod(mCurrentTime, mCurrentAnimation->GetDuration());
		}
		else {
			if (mCurrentTime >= std::nextafter(mCurrentAnimation->GetDuration(), 0.0f)) {
				mCurrentTime = std::nextafter(mCurrentAnimation->GetDuration(), 0.0f);
				mIsFinished = true;
			}
		}
		CalculateBoneTransform(&mCurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		for (int i = 0; i < kMaxBones; i++) {
			matrix[i] = GlmConvert::ConvertGLMtoMatrix(mFinalBoneMatrices[i]);
		}
	}
}

void Animator::PlayAnimation(Animation* pAnimation, bool loopFlag)
{
	//アニメーションが同じではないかつアニメーションが終わっているときに変更する
	if (mCurrentAnimation != pAnimation||mIsFinished) {
		mCurrentAnimation = pAnimation;
		mCurrentTime = 0.0f;
		mIsLooped = loopFlag;
		mIsFinished = false;
	}
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

	auto& boneInFoMap = mCurrentAnimation->GetBoneIDMap();
	if (boneInFoMap.find(nodeName) != boneInFoMap.end()) {
		int index = boneInFoMap.find(nodeName)->second.id;
		glm::mat4 offset = boneInFoMap.find(nodeName)->second.offset;
		mFinalBoneMatrices[index] = globalTransformation * offset;
		mGlobalBoneMatrices[index] = globalTransformation;
	}
	for (int i = 0; i < node->childrenCount; i++) {
		CalculateBoneTransform(&node->children[i], globalTransformation);
	}
}

Matrix4x4 Animator::GetBoneWorldMatrix(
	int boneIndex, const Matrix4x4 worldMatrix)const {
	if (boneIndex < 0) {
		return worldMatrix;
	}

	const Matrix4x4 boneMatrix = GlmConvert::ConvertGLMtoMatrix(mGlobalBoneMatrices[boneIndex]);
	return Multiply(boneMatrix, worldMatrix);
}

