#include "Animation.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h> 
#include <assimp/scene.h>
#include "Bone.h"

Animation::~Animation()
{}

void Animation::Initialize(
	const std::string& animationPath,
	SkinnedModel* skinnedModel)
{
	mSkinnedModel = skinnedModel;
	Assimp::Importer importer{};
	const aiScene* scene = importer.ReadFile(
		animationPath, 
		aiProcess_ConvertToLeftHanded
	);

	assert(scene && scene->mRootNode);
	auto animation = scene->mAnimations[0];
	mDuration =(float) animation->mDuration;
	mTickPerSecond = (int)animation->mTicksPerSecond;
	ReadHeirarchyData(mRootNode, scene->mRootNode);
	ReadMissingBones(animation);
}

Bone* Animation::FindBone(const std::string& name) {
	auto iter = std::find_if(mBones.begin(), mBones.end(),
		[&](const Bone& Bone) {
			return Bone.GetBoneName() == name;
		}
	);

	if (iter == mBones.end())return nullptr;
	else return &(*iter);
}

	void Animation::ReadMissingBones(const aiAnimation* animation) {
		int size = animation->mNumChannels;
		const auto& boneInFoMap = mSkinnedModel->GetBoneInFoMap();
		int boneCount = mSkinnedModel->GetBoneCount();
		mBoneInFoMap = boneInFoMap;
		for (int i = 0; i < size; i++)
		{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;
			Bone bone{};
			if (mBoneInFoMap.find(boneName) == mBoneInFoMap.end())
			{
				mBoneInFoMap[boneName].id = boneCount;
				boneCount++;
			}
			bone.Initialize(channel->mNodeName.data,
				mBoneInFoMap[channel->mNodeName.data].id, channel);
		
			mBones.push_back(std::move(bone));
		}
	}

	void Animation::ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src) {
		assert(src);
		dest.name = src->mName.data;
		dest.transformation = GlmConvert::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount = src->mNumChildren;
		for (int i = 0; i < src->mNumChildren; i++) {
			AssimpNodeData newData;
			ReadHeirarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}
