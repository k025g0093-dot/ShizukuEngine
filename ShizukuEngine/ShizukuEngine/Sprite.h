#pragma once
#include "Model.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;   // ★追加

struct Material;

class Sprite : public Model {
public:
    Sprite();
    ~Sprite() override;

    void InitSprite(ComPtr<ID3D12Device> device);

    void SetUVTransform(const Matrix4x4& uvTransform) override;


    void Draw(ID3D12GraphicsCommandList* cmdList,
        UINT instanceCount,
        UINT startInstanceLocation)override;

    int GetTextureIndex() const { return mTextureIndex; }

private:

    int   mTextureIndex = 0;
    float mWidth = 0.0f;
    float mHeight = 0.0f;

    ComPtr<ID3D12Resource>   mPVertexResource;   // ★
    ComPtr<ID3D12Resource>   mPMaterialResource; // ★
    ComPtr<ID3D12Resource>   mPWvpResource;      // ★

    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};

    uint32_t Align256(uint32_t size)
    {
        return (size + 0xff) & ~0xff;
    }
};
