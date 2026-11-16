#pragma once
#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include "AssimpGLMHelpersMath/GlmConvert.h"
#include "SkinnedModel.h"

struct aiAnimation;
class Bone;

struct AssimpNodeData
{

	glm::mat4 transformation;//トランスフォー
	std::string name;//名前
	int childrenCount;//子ノード数
	std::vector<AssimpNodeData> children;//子ノード

};

class Animation
{
public:
	Animation() = default;
	~Animation();
	void Initialize(const std::string& animationPath, SkinnedModel* skinnedModel);

	Bone* FindBone(const std::string& name);

	float GetTicksPerSecond() { return (float)mTickPerSecond; }
	float GetDuration() { return mDuration; }
	const AssimpNodeData& GetRootNode() { return mRootNode; }
	const std::map<std::string, BoneInFo>& GetBoneIDMap() { return mBoneInFoMap; }

private://プライベート関数

	void ReadMissingBones(const aiAnimation* animation);

	void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);

private://メンバ変数

	SkinnedModel *mSkinnedModel=nullptr;

	float mDuration;
	int mTickPerSecond;
	std::vector<Bone>mBones;
	AssimpNodeData mRootNode;
	std::map<std::string, BoneInFo>mBoneInFoMap;
};

