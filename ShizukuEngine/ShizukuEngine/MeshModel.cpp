#include "MeshModel.h"
#pragma comment(lib,"assimp-vc145-mt")


MeshModel::MeshModel() = default;
MeshModel::~MeshModel() = default;

//アシンプのローダー関数
const aiScene* MeshModel::Load(const std::string& filepath, Assimp::Importer& importer) {
	const aiScene* scene = importer.ReadFile(filepath,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_ValidateDataStructure |
		aiProcess_ImproveCacheLocality |
		aiProcess_SortByPType |
		aiProcess_ConvertToLeftHanded);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
		return nullptr;
	}

	return scene;
}

//モデル読み込み
bool MeshModel::LoadFromModel(
	ComPtr<ID3D12Device> device,
	const std::string& filepath
) {

	//ファイルが合っているか確認、間違っていたりすればfalseを返す
	Assimp::Importer importer;
	const aiScene* scene = Load(filepath, importer);
	if (!scene || !scene->mRootNode)return false;

	//頂点インデックスを集める
	std::vector<VertexData>vertices;
	std::vector<uint32_t>indices;

	ProcessNode(scene->mRootNode, scene, vertices, indices); // 再帰トラバース
	CreateBuffers(device, vertices,indices);

	mPMaterialResource = CreateBufferResource(device.Get(), Align256(sizeof(Material)));
	Material* materialData = nullptr;
	mPMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableNormalMap = 0;
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();
	mPMaterialResource->Unmap(0, nullptr);

	return true;

}

void MeshModel::Draw(ID3D12GraphicsCommandList* cmdList, UINT instanceCount, UINT startInstanceLocation) {
	cmdList->IASetVertexBuffers(0, 1, &mVertexBufferView);
	cmdList->IASetIndexBuffer(&mIndexBufferView);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	cmdList->SetGraphicsRootConstantBufferView(0, mPMaterialResource->GetGPUVirtualAddress());

	cmdList->DrawIndexedInstanced(mIndexCount, instanceCount, 0, 0, startInstanceLocation);
}


void MeshModel::ProcessNode(aiNode* node, const aiScene* scene,
	std::vector<VertexData>& vertices, std::vector<uint32_t>& indices,
	const aiMatrix4x4& parentTransform)
{
	aiMatrix4x4 nodeTransform = parentTransform * node->mTransformation;

	for (UINT i = 0; i < node->mNumMeshes; i++) {
		unsigned int meshIndex = node->mMeshes[i];
		if (meshIndex >= scene->mNumMeshes) continue;
		aiMesh* mesh = scene->mMeshes[meshIndex];
		if (!mesh || !mesh->mVertices) continue;
		UINT baseVertex = (UINT)vertices.size();
		for (UINT v = 0; v < mesh->mNumVertices; v++) {
			VertexData vert{};
			aiVector3D pos = nodeTransform * mesh->mVertices[v];
			vert.position = { pos.x, pos.y, pos.z, 1.0f };
			if (mesh->mTextureCoords[0]) {
				vert.texcord = { mesh->mTextureCoords[0][v].x, 1.0f - mesh->mTextureCoords[0][v].y };
			}
			if (mesh->mNormals) {
				aiVector3D nm = aiMatrix3x3(nodeTransform) * mesh->mNormals[v];
				vert.normal = { nm.x, nm.y, nm.z };
			}
			if (mesh->mTangents) {
				vert.tangent = { mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z };
			}
			vertices.push_back(vert);
		}
		for (UINT f = 0; f < mesh->mNumFaces; f++) {
			aiFace& face = mesh->mFaces[f];
			for (UINT idx = 0; idx < face.mNumIndices; idx++)
				indices.push_back(baseVertex + face.mIndices[idx]);
		}
	}
	for (UINT i = 0; i < node->mNumChildren; i++)
		if (node->mChildren[i])
			ProcessNode(node->mChildren[i], scene, vertices, indices, nodeTransform);
}


//バッファの作成関数
void MeshModel::CreateBuffers(
	ComPtr<ID3D12Device> device,
	std::vector<VertexData>& vertices,
	std::vector<uint32_t>& indices
){
	mVertexCount = (uint32_t)vertices.size();
	mIndexCount = (uint32_t)indices.size();

	// 頂点バッファ
	mPVertexResource = CreateBufferResource(device.Get(), sizeof(VertexData) * mVertexCount);
	void* pData = nullptr;
	mPVertexResource->Map(0, nullptr, &pData);
	memcpy(pData, vertices.data(), sizeof(VertexData) * mVertexCount);
	mPVertexResource->Unmap(0, nullptr);

	mVertexBufferView.BufferLocation = mPVertexResource->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = sizeof(VertexData) * (UINT)mVertexCount;
	mVertexBufferView.StrideInBytes = sizeof(VertexData);


	// インデックスバッファ
	mPIndexResource = CreateBufferResource(device.Get(), sizeof(uint32_t) * mIndexCount);
	uint32_t* idxData = nullptr;
	mPIndexResource->Map(0, nullptr, reinterpret_cast<void**>(&idxData));
	memcpy(idxData, indices.data(), sizeof(uint32_t) * mIndexCount);
	mPIndexResource->Unmap(0, nullptr);

	mIndexBufferView.BufferLocation = mPIndexResource->GetGPUVirtualAddress();
	mIndexBufferView.SizeInBytes = sizeof(uint32_t) * mIndexCount;
	mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
}
