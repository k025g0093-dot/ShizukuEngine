#pragma once
#include "Model.h"
#include <wrl.h>
#include <d3d12.h>

using Microsoft::WRL::ComPtr;

struct VertexData;
struct Material;

class TriangleModel :public Model //基底クラスModelを参照
{
public:
	void Initialization(ComPtr<ID3D12Device> device);



    void Draw(
        ID3D12GraphicsCommandList* cmdList,
        UINT instanceCount,
        UINT startInstanceLocation)override;



private://メンバ変数
    ComPtr<ID3D12Resource>   mPVertexResource;
    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
    ComPtr<ID3D12Resource>   mPMaterialResource;
    ComPtr<ID3D12Resource>   mPWvpResource;
    ComPtr<ID3D12Resource>   mPLightResource;   // 追加
    Material* materialData = nullptr;


    uint32_t Align256(uint32_t size) {
        return (size + 0xff) & ~0xff;
    }
};


