#pragma once
#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <wrl.h>
#include <vector>
#include <fstream>
#include <sstream>

using Microsoft::WRL::ComPtr;

struct VertexData;

struct MaterialData
{
    std::string textureFilPath;
    std::string normalTextureFilePath;
};

struct ModelData
{
    std::vector<VertexData> vertices;
    MaterialData material;
};

class MeshModel : public Model 
{   

public://パブリック
	MeshModel();
	~MeshModel();


	bool LoadFromModel(ComPtr<ID3D12Device> device, const std::string& filepath);

    void Draw(
        ID3D12GraphicsCommandList* cmdList,
        UINT instanceCount,
        UINT startInstanceLocation) override;

private://プライベート関数
    
    static const aiScene* Load(const std::string& filepath, Assimp::Importer& importer);
    
    void ProcessNode(aiNode* node, const aiScene* scene,
        std::vector<VertexData>& vertices, std::vector<uint32_t>& indices,
        const aiMatrix4x4& parentTransform = aiMatrix4x4());

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

    uint32_t Align256(uint32_t size)
    {
        return (size + 0xff) & ~0xff;
    }

};
