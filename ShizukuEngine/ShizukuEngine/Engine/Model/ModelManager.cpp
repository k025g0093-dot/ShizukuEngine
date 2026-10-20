#include "ModelManager.h"
#include "Core/ConvertString.h"
#include <fstream>
#include <filesystem>
#include"TextureManager/TextureManager.h"

int ModelManager::Load(const std::string& filePath) {

	const auto key =std::filesystem::path(filePath).lexically_normal().generic_string() ;   //  追加

	if (mPathToIndex.count(key) > 0) {//  filename → key
		return mPathToIndex[key];//  filename → key
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