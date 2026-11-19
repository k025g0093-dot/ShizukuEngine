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

OBB Animator::UpdateOBBAnimator(
	const OBB& obb,
	Matrix4x4 worldMatrix
) {
	OBB result{};
	result.center = TransformMatrix(obb.center, worldMatrix);

	const float sizes[3] = { obb.size.x,obb.size.y, obb.size.z };

	float scaled[3]{};
	for (int i = 0; i < 3; i++) {
		const Vector3& d = obb.orientations[i];
		const Vector3 axis = {
			d.x * worldMatrix.m[0][0] + d.y * worldMatrix.m[1][0] + d.z * worldMatrix.m[2][0],
			d.x * worldMatrix.m[0][1] + d.y * worldMatrix.m[1][1] + d.z * worldMatrix.m[2][1],
			d.x * worldMatrix.m[0][2] + d.y * worldMatrix.m[1][2] + d.z * worldMatrix.m[2][2],
		};
		const float length = axis.Length();
		result.orientations[i] = axis.Normalized();
		scaled[i] = sizes[i] * length;
	}

	result.size = { scaled[0],scaled[1] ,scaled[2] };
	return result;
}
