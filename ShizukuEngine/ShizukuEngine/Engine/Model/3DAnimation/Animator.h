#pragma once
#include "Animation.h"
#include "Bone.h"
#include <Math/Vector.h>

class Animator
{

public:
	
	void Initialize(Animation* animation);

	void UpdateAnimation(float dt);

	void PlayAnimation(Animation* pAnimation);

	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);

	std::vector<glm::mat4> GetFinalBoneMatrices() {
		return mFinalBoneMatrices;
	}

	const std::vector<Matrix4x4>& GetAnimationMatrix() {
		return matrix;
	}

private:
	
	void UpdateOBBAnimator(const std::vector<OBB>& boneObb,Matrix4x4 worldMatrix, std::vector<OBB>&resultObb);

private:
	std::vector<Matrix4x4> matrix;
	std::vector<glm::mat4>mFinalBoneMatrices;
	std::vector<glm::mat4>mGlobalBoneMatrices;
	Animation* mCurrentAnimation;
	float mCurrentTime;
	float mDeltaTime;

};

