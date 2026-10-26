#pragma once
#include "../SkinningConstants.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "Model/Model.h"
# include <map>
//assimpの前方宣言
namespace Assimp {
	class Importer; // Importer クラス
}
struct aiScene;//モデル全体の情報
struct aiMesh;//メッシュ情報
struct aiNode;//ノードの情報
struct aiMaterial; //マテリアル情報
class GlmConvert;

struct Vertex {
	//position
	glm::vec3 position;
	//normal
	glm::vec3 normal;
	//texCoords
	glm::vec2 texCoords;
	//tangent
	glm::vec3 tangent;
	//biTangent
	glm::vec3 biTangent;
	int mBoneIds[kMaxBoneIngluence];
	float mWeights[kMaxBoneIngluence];
};

struct BoneInFo {
	int id;
	glm::mat4 offset;
};

class SkinnedModel:public Model
{

public:

	SkinnedModel();
	~SkinnedModel();

	//ファイル取得を行うload関数
	bool LoadSkinnedModel(ID3D12Device* device, const std::string& filepath);

	void Draw(
		ID3D12GraphicsCommandList* cmdList,
		UINT instanceCount,
		UINT startInstanceLocation) override;

	void TexPath(const aiMaterial* mtl, const std::string& filepath)override;

	//マテリアルの色（シェーダーのgMaterial.colorに届く。同じモデルを使う全インスタンスに効く）
	void SetColor(const Vector4& color);
	const Vector4& GetColor() const { return mColor; }

	//ライティングを受けるかどうか
	void SetEnableLighting(bool enableLighting);
	bool GetEnableLighting() const { return mEnableLighting; }

private://関数

	void SetVertexBoneDataToDefault(Vertex& vertex);

	static const aiScene* Load(const std::string& filepath, Assimp::Importer& importer);

	void ProcessMesh(aiMesh* mesh, const aiScene* scene);

	void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);

	void SetVertexBoneData(Vertex& vertex, int boneID, float weight);

	auto& GetBoneInFoMap() { return mBoneInfoMap; }
	int& GetBoneCount() { return mBoneCounter; }

	//mColorなどの値をマテリアルのバッファに書き込む
	void UpdateMaterial();

	void CreateBuffers(
		ID3D12Device* device,
		std::vector<Vertex>& vertices,
		std::vector<uint32_t>& indices);

private://メンバ変数

	//頂点バッファ
	ComPtr<ID3D12Resource>   mPVertexResource;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};

	//インデックスバッファ
	ComPtr<ID3D12Resource>   mPIndexResource;
	D3D12_INDEX_BUFFER_VIEW  mIndexBufferView{};

	//マテリアル
	ComPtr<ID3D12Resource>   mPMaterialResource;

	int mTextureIndex = -1;

	std::string mTexturePath;

	//マテリアルの値（UpdateMaterialでバッファに反映する）
	Vector4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool mEnableLighting = true;

	uint32_t Align256(uint32_t size)
	{
		return (size + 0xff) & ~0xff;
	}


	std::map<std::string, BoneInFo>mBoneInfoMap;
	int mBoneCounter = 0;

	std::vector<Vertex> mVertices;
	std::vector<unsigned int>mIndices;

};

