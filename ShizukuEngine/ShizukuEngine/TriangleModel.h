#pragma once
#include "Model.h"
#include <wrl.h>
#include <d3d12.h>

using Microsoft::WRL::ComPtr;

struct VertexData;
struct Material;

class TraiangleModel :public Model //基底クラスModelを参照
{

	void Initialaize(ComPtr<ID3D12Device>* device);

    void UpdateVertices(
        const Vector3& positions,
        const Vector2& texcoord,
        const Vector3& normal,
        int index) override;

    void Draw(
        ID3D12GraphicsCommandList* cmdList,
        int textureIndex,
        UINT instanceCount,
        UINT startInstanceLocation)override;


private:
    ComPtr<ID3D12Resource>   mPVertexResource;
    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
    ComPtr<ID3D12Resource>   mPMaterialResource;
    ComPtr<ID3D12Resource>   mPWvpResource;
    ComPtr<ID3D12Resource>   mPLightResource;   // 追加
    Material* materialData = nullptr;

    VertexData* m_pVertexData = nullptr;

    uint32_t Align256(uint32_t size) {
        return (size + 0xff) & ~0xff;
    }
};


