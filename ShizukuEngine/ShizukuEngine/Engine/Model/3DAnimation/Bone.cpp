#include "Bone.h"
#include <assimp/scene.h>
#pragma comment(lib,"assimp-vc145-mt")
Bone::~Bone() {}

void Bone::Initialize(const std::string& name, int ID, const aiNodeAnim* channel) {

	mName=name;
	mID = ID;
	mLocalTransform=1.0f;

	/*position関係*/
	mNumPositions = channel->mNumPositionKeys;
	for (int positionIndex = 0; positionIndex < mNumPositions; positionIndex++) {
		//アニメーションのキーを格納
		aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
		//キーの中に格納されてる時間を取得
		float timeStamp = (float)channel->mPositionKeys[positionIndex].mTime;
		KeyPosition data{};
		//glm型のVecに変更して代入
		data.position = GlmConvert::GetGLMVec(aiPosition);
		data.timeStamp = timeStamp;
		mPosition.push_back(data);
	}
	/*rotate関係*/
	mNumRotations = channel->mNumRotationKeys;
	for (int rotationIndex = 0; rotationIndex < mNumRotations; rotationIndex++) {

		//クオータニオンで宣言
		aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
		//キーの中に格納されてる時間を取得
		float timeStamp = channel->mRotationKeys[rotationIndex].mTime;

		KeyRotation data{};
		//glm型のQuatに変更して代入
		data.orientation= GlmConvert::GetGLMQuat(aiOrientation);
		data.timeStamp = timeStamp;
		mRotation.push_back(data);
	}
	/*scale関係*/
	mNumScalings = channel->mNumScalingKeys;
	for (int keyIndex = 0; keyIndex < mNumScalings; keyIndex++) {

		aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
		//キーの中に格納されてる時間を取得
		float timeStamp = channel->mRotationKeys[keyIndex].mTime;

		KeyScale data{};
		data.scale = GlmConvert::GetGLMVec(scale);
		data.timeStamp = timeStamp;
		mScale.push_back(data);
	}
}

int Bone::GetPositionIndex(float animationTime)const {
	for (int index = 0; index < mNumPositions-1; index++) {
		if (animationTime < mPosition[index + 1].timeStamp) {
			return index;
		}
	}
	assert(0);
}

int Bone::GetRotationIndex(float animationTime)const {
	for (int index = 0; index < mNumRotations-1; index++) {
		if (animationTime < mRotation[index + 1].timeStamp) {
			return index;
		}
	}
	assert(0);
}

int Bone::GetScaleIndex(float animationTime) const {
	for (int index = 0; index < mNumScalings-1; index++) {
		if (animationTime < mScale[index + 1].timeStamp) {
			return index;
		}
	}
	assert(0);

}

float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) const {
	float scaleFactor = 0.0f;
	float midWayLength = animationTime - lastTimeStamp;
	float framesDiff = nextTimeStamp - lastTimeStamp;
	scaleFactor = midWayLength / framesDiff;
	return scaleFactor;
}

glm::mat4 Bone::InterpolatePosition(float animationTime) const {
	if (1 == mNumPositions)
		return glm::translate(glm::mat4(1.0f), mPosition[0].position);
	int p0Index = GetPositionIndex(animationTime);
	int p1Index = p0Index + 1;
	
	//スケールファクターを宣言
	float scaleFactor = GetScaleFactor(
		mPosition[p0Index].timeStamp,
		mPosition[p1Index].timeStamp,
		animationTime
	);

	//アニメーションの補間する際の最終的な位置を計算
	glm::vec3 finalPosition = glm::mix(mPosition[p0Index].position,
		mPosition[p1Index].position, scaleFactor
	);

	return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::InterpolateRotation(float animationTime) const {
	if (1 == mNumPositions) {
		auto rotation = glm::normalize(mRotation[0].orientation);
		return glm::mat4_cast(rotation);
	}
	
	int p0Index = GetRotationIndex(animationTime);
	int p1Index = p0Index + 1;

	//スケールファクターを宣言
	float scaleFactor = GetScaleFactor(
		mPosition[p0Index].timeStamp,
		mPosition[p1Index].timeStamp,
		animationTime
	);

	//アニメーションの補間する際の最終的な位置を計算
	glm::quat finalRotation = glm::slerp(mRotation[p0Index].orientation,
		mRotation[p1Index].orientation, scaleFactor
	);

	finalRotation = glm::normalize(finalRotation);

	return glm::mat4_cast(finalRotation);
}

glm::mat4 Bone::InterpolateScaling(float animationTime)const {
	if (1 == mNumScalings)
		return glm::scale(glm::mat4(1.0f), mScale[0].scale);
	int p0Index = GetScaleIndex(animationTime);
	int p1Index = p0Index + 1;

	//スケールファクターを宣言
	float scaleFactor = GetScaleFactor(
		mScale[p0Index].timeStamp,
		mScale[p1Index].timeStamp,
		animationTime
	);

	//アニメーションの補間する際の最終的な位置を計算
	glm::vec3 finalScale = glm::mix(mScale[p0Index].scale,
		mScale[p1Index].scale, scaleFactor
	);

	return glm::scale(glm::mat4(1.0f), finalScale);
}

glm::mat4 Bone::CalculateLocalTransform(float animationTime)const {
	glm::mat4 result{};
	glm::mat4 translation = InterpolatePosition(animationTime);
	glm::mat4 rotation = InterpolateRotation(animationTime);
	glm::mat4 scale = InterpolateScaling(animationTime);
	result = translation * rotation * scale;
	return result;
}
