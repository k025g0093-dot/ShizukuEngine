#include "ModelManager.h"
#include "Core/ConvertString.h"
#include <fstream>
#include <filesystem>
#include"TextureManager/TextureManager.h"

int ModelManager::Load(const std::string& filePath) {

	const auto key =std::filesystem::path(filePath).lexically_normal().generic_string() ; 

	if (mPathToIndex.count(key) > 0) {
		return mPathToIndex[key];
	}

	auto mesh = std::make_unique<MeshModel>();

	if (!mesh->LoadFromModel(mDevice, filePath)) {
		OutputDebugStringA(("Error: Failed to load model: " + filePath + "\n").c_str());	
		return -1;
	}

	assert(mTextureManager);
	const std::string texPath = mesh->GetTexturePath();

	const int texIndex = mTextureManager->LoadTexture(texPath);
	mesh->SetTextureIndex(texIndex);

	int index = static_cast<int>(mModels.size());
	mModels.push_back(std::move(mesh));

	mPathToIndex[key] = index;
	return index;

}


//スキンモデルの読み込み
int ModelManager::LoadSkinnedModel(const std::string& filePath) {
	const auto key = std::filesystem::path(filePath).lexically_normal().generic_string();

	if (mPathToIndex.count(key) > 0) {
		return mPathToIndex[key];
	}

	auto skinnedModel = std::make_unique<SkinnedModel>();

	if (!skinnedModel->LoadSkinned(mDevice, filePath)) {
		OutputDebugStringA(("Error: Failed to load model: " + filePath + "\n").c_str());//ファイルがないことを伝え
		return -1;//-1でリターンを行う
	}

	//テクスチャマネージャーがない場合はassertで知らせる
	assert(mTextureManager);
	const std::string texPath = skinnedModel->GetTexturePath();

	const int texIndex = mTextureManager->LoadTexture(texPath);
	skinnedModel->SetTextureIndex(texIndex);

	int  index = static_cast<int>(mSkinnedModel.size());
	mSkinnedModel.push_back(std::move(skinnedModel));

	mPathToIndex[key] = index;
	return index;

}

void ModelManager::Initialize(ID3D12Device* device, TextureManager* textureManager) {
	mDevice = device;
	mTextureManager=textureManager;
}


MeshModel* ModelManager::Get(int index) const{
	if (index < 0 || index >= mModels.size()) {
		return nullptr;
	}
	return mModels[index].get();
}

SkinnedModel* ModelManager::GetSkinned(int index)const {

	if (index < 0 || index >= mSkinnedModel.size()) {
		return nullptr;
	}
	return mSkinnedModel[index].get();

}



