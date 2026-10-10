#pragma once
#include "Model.h"
#include <wrl.h>
#include <d3d12.h>

using Microsoft::WRL::ComPtr;

struct VertexData;
struct Material;

class Sphere : public Model {
public:
    Sphere() = default;
    ~Sphere() override;

    void Initialization(ComPtr<ID3D12Device> device);
    void Update();

    void Draw(
        ID3D12GraphicsCommandList* cmdList,
        UINT instanceCount,
        UINT startInstanceLocation) override;

    void SetLightResource(ID3D12Resource* lightResource);

private:
    ComPtr<ID3D12Resource>   mPVertexResource;
    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};

    ComPtr<ID3D12Resource>   mPIndexResource;
    D3D12_INDEX_BUFFER_VIEW  mIndexBufferView{};

    ComPtr<ID3D12Resource>   mPMaterialResource;
    ComPtr<ID3D12Resource>   mPWvpResource;



    uint32_t Align256(uint32_t size)
    {
        return (size + 0xff) & ~0xff;
    }
};