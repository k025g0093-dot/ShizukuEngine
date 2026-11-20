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

	Matrix4x4 GetBoneWorldMatrix(int boneIndex,const Matrix4x4 worldMatrix)const;

private:
	

private:
	std::vector<Matrix4x4> matrix;
	Matrix4x4 mWorldMatrix;

	std::vector<glm::mat4>mFinalBoneMatrices;
	std::vector<glm::mat4>mGlobalBoneMatrices;
	Animation* mCurrentAnimation;
	float mCurrentTime;
	float mDeltaTime;

};

