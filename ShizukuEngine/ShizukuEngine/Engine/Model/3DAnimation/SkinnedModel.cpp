#include "SkinnedModel.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma comment(lib,"assimp-vc145-mt")
#include "AssimpGLMHelpersMath/GlmConvert.h"


using namespace std;

SkinnedModel::SkinnedModel() {}
SkinnedModel::~SkinnedModel() {}

const aiScene* SkinnedModel::Load(const std::string& filepath, Assimp::Importer& importer) {
	const aiScene* scene = importer.ReadFile(filepath,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_ValidateDataStructure |
		aiProcess_ImproveCacheLocality |
		aiProcess_SortByPType |
		aiProcess_LimitBoneWeights |
		aiProcess_ConvertToLeftHanded);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
		return nullptr;
	}
	return scene;

}

bool SkinnedModel::LoadSkinnedModel(
	ID3D12Device* device,
	const std::string& filepath
) {
	//ファイルが合っていない場合はfalseを返す
	Assimp::Importer importer;
	const aiScene* scene = Load(filepath, importer);
	if (!scene || !scene->mRootNode)return false;
	//スキン用の構造体を使用
	std::vector<Vertex>vertices;
	std::vector<uint32_t>indices;
	ProcessMesh(*scene->mMeshes, scene);
	CreateBuffers(device, vertices, indices);
	if (scene->mNumMeshes > 0) {
		const unsigned int materialIndex = scene->mMeshes[0]->mMaterialIndex;
		if (materialIndex < scene->mNumMaterials) {
			TexPath(scene->mMaterials[materialIndex], filepath);
		}
	}
	mPMaterialResource = CreateBufferResource(device, Align256(sizeof(Material)));
	UpdateMaterial();

	return true;
}


void SkinnedModel::Draw(
	ID3D12GraphicsCommandList* cmdList,
	UINT instanceCount, 
	UINT startInstanceLocation) {

	cmdList->IASetVertexBuffers(0, 1, &mVertexBufferView);
	cmdList->IASetIndexBuffer(&mIndexBufferView);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->SetGraphicsRootConstantBufferView(0, mPMaterialResource->GetGPUVirtualAddress());
	cmdList->DrawIndexedInstanced(mIndexCount, instanceCount, 0, 0, startInstanceLocation);

}

void SkinnedModel::SetVertexBoneDataToDefault(Vertex& vertex) {
	for (int i = 0; i < kMaxBoneIngluence; i++) {
		vertex.mBoneIds[i] = -1;
		vertex.mWeights[i] = 0.0f;
	}
}

void SkinnedModel::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		SetVertexBoneDataToDefault(vertex);
		vertex.position = GlmConvert::GetGLMVec(mesh->mVertices[i]);
		vertex.normal = GlmConvert::GetGLMVec(mesh->mNormals[i]);

		if (mesh->mTextureCoords[0]) {
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.texCoords = vec;
		}
		else {
			vertex.texCoords = glm::vec2(0.0f, 0.0f);
		}
		mVertices.push_back(vertex);
		ExtractBoneWeightForVertices(mVertices, mesh, scene);
	}
}

void SkinnedModel::SetVertexBoneData(Vertex& vertex, int boneID, float weight)
{
	for (int i = 0; i < kMaxBoneIngluence; ++i)
	{
		if (vertex.mBoneIds[i] < 0)
		{
			vertex.mWeights[i] = weight;
			vertex.mBoneIds[i] = boneID;
			break;
		}
	}
}

void SkinnedModel::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
{
	for (int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {

		int boneID = -1;
		std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
		if (mBoneInfoMap.find(boneName) == mBoneInfoMap.end()) {

			BoneInFo newBoneInfo;
			newBoneInfo.id = mBoneCounter;
			newBoneInfo.offset = GlmConvert::ConvertMatrixToGLMFormat(
				mesh->mBones[boneIndex]->mOffsetMatrix);
			mBoneInfoMap[boneName] = newBoneInfo;
			boneID = mBoneCounter;
			mBoneCounter++;
		}
		else {
			boneID = mBoneInfoMap[boneName].id;
		}
		assert(boneID != -1);
		auto weights = mesh->mBones[boneIndex]->mWeights;
		int numWeights = mesh->mBones[boneIndex]->mNumWeights;

		for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
			int vertexId = weights[weightIndex].mVertexId;
			float weight = weights[weightIndex].mWeight;
			assert(vertexId <= vertices.size());
			SetVertexBoneData(vertices[vertexId], boneID, weight);
		}
	}
}

void SkinnedModel::CreateBuffers(
	ID3D12Device* device,
	std::vector<Vertex>& vertices,
	std::vector<uint32_t>& indices
) {
	mVertexCount = (uint32_t)vertices.size();
	mIndexCount = (uint32_t)indices.size();

	//頂点バッファ
	mPVertexResource = CreateBufferResource(device, sizeof(Vertex) * mVertexCount);
	void* pData = nullptr;
	mPVertexResource->Map(0, nullptr, &pData);
	memcpy(pData, vertices.data(), sizeof(Vertex) * mVertexCount);
	mPVertexResource->Unmap(0, nullptr);

	mVertexBufferView.BufferLocation = mPVertexResource->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = sizeof(Vertex) * (UINT)mVertexCount;
	mVertexBufferView.StrideInBytes = sizeof(Vertex);

	//インデックスバッファー
	mPIndexResource = CreateBufferResource(device, sizeof(uint32_t) * mIndexCount);
	uint32_t* idxData = nullptr;
	mPIndexResource->Map(0, nullptr, reinterpret_cast<void**>(&idxData));
	memcpy(idxData, indices.data(), sizeof(uint32_t) * mIndexCount);
	mPIndexResource->Unmap(0, nullptr);

	mIndexBufferView.BufferLocation = mPIndexResource->GetGPUVirtualAddress();
	mIndexBufferView.SizeInBytes = sizeof(uint32_t) * mIndexCount;
	mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

void SkinnedModel::TexPath(const aiMaterial* mtl, const std::string& filepath) {
	if (!mtl) {
		return;
	}
	aiString path;
	if (mtl->GetTexture(aiTextureType_DIFFUSE, 0, &path) != AI_SUCCESS) { return; }
	std::filesystem::path texPath = path.C_Str();
	// 相対パスなら、モデルファイルがあるフォルダから見たパスにする
	if (texPath.is_relative()) {
		texPath = std::filesystem::path(filepath).parent_path() / texPath;
	}
	mTexturePath = texPath.lexically_normal().generic_string();
}

//マテリアルの色を変える
void SkinnedModel::SetColor(const Vector4& color) {
	mColor = color;
	UpdateMaterial();
}

//ライティングの有無を変える
void SkinnedModel::SetEnableLighting(bool enableLighting) {
	mEnableLighting = enableLighting;
	UpdateMaterial();
}

//メンバの値をマテリアルのバッファに書き込む
void SkinnedModel::UpdateMaterial() {
	if (!mPMaterialResource) { return; }

	Material* materialData = nullptr;
	mPMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = mColor;
	materialData->enableNormalMap = 0;
	materialData->enableLighting = mEnableLighting ? 1 : 0;
	materialData->uvTransform = MakeIdentity4x4();
	mPMaterialResource->Unmap(0, nullptr);
}
