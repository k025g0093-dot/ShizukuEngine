#pragma once
#include "Model.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;   // ★追加

struct Material;

class Sprite : public Model {
public:
    Sprite();
    ~Sprite() override;

    void Initialization(ComPtr<ID3D12Device> device);



    void Draw(ID3D12GraphicsCommandList* cmdList,
        UINT instanceCount,
        UINT startInstanceLocation)override;


private:


    ComPtr<ID3D12Resource>   mPVertexResource;   
    ComPtr<ID3D12Resource>   mPMaterialResource; 

    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};

    uint32_t Align256(uint32_t size)
    {
        return (size + 0xff) & ~0xff;
    }
};
