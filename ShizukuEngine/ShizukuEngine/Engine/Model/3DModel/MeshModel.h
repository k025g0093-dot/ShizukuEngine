#pragma once
#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <wrl.h>
#include <vector>


using Microsoft::WRL::ComPtr;

struct VertexData;



class MeshModel : public Model 
{   

public://パブリック
	MeshModel();
	~MeshModel();


	bool LoadFromModel(ID3D12Device *device, const std::string& filepath);

    void Draw(
        ID3D12GraphicsCommandList* cmdList,
        UINT instanceCount,
        UINT startInstanceLocation) override;

    void TexPath(const aiMaterial * mtl,const std::string& filepath);

    const std::string GetTexturePath() const{ return  mTexturePath; }

    //テクスチャーインデックスゲッター
    const int GetTextureIndex()const { return mTextureIndex; }
    //テクスチャーインデックスセッター
    void SetTextureIndex(int textureIndex) {
        mTextureIndex = textureIndex;
    }

    //マテリアルの色（シェーダーのgMaterial.colorに届く。同じモデルを使う全インスタンスに効く）
    void SetColor(const Vector4& color);
    const Vector4& GetColor() const { return mColor; }

    //ライティングを受けるかどうか
    void SetEnableLighting(bool enableLighting);
    bool GetEnableLighting() const { return mEnableLighting; }

private://プライベート関数
    
    static const aiScene* Load(const std::string& filepath, Assimp::Importer& importer);
    
    void ProcessNode(aiNode* node, const aiScene* scene,
        std::vector<VertexData>& vertices, std::vector<uint32_t>& indices,
        const aiMatrix4x4& parentTransform = aiMatrix4x4());

    //mColorなどの値をマテリアルのバッファに書き込む
    void UpdateMaterial();

    void CreateBuffers(
        ComPtr<ID3D12Device> device,
        std::vector<VertexData>& vertices,
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

};
