#include "Sprite.h"
Sprite::Sprite() = default;
Sprite::~Sprite() = default;

void Sprite::Initialization(ComPtr<ID3D12Device> device) {


    // --- 頂点バッファ ---
    mPVertexResource = CreateBufferResource(device.Get(), sizeof(VertexData) * 4);
    mPVertexResource->Map(0, nullptr, reinterpret_cast<void**>(&mPVertexData));

    mVertexBufferView.BufferLocation = mPVertexResource->GetGPUVirtualAddress();
    mVertexBufferView.StrideInBytes = sizeof(VertexData);
    mVertexBufferView.SizeInBytes = sizeof(VertexData) * 4;

    VertexData* vertexData = nullptr;
    mPVertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

    // TRIANGLESTRIPの順番：左上 → 右上 → 左下 → 右下
    vertexData[0] = { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } }; // 左上
    vertexData[1] = { { 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } }; // 右上
    vertexData[2] = { { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } }; // 左下
    vertexData[3] = { { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } }; // 右下


    mPVertexResource->Unmap(0, nullptr);

    // --- マテリアルバッファ ---
    mPMaterialResource = CreateBufferResource(device.Get(),sizeof(Material));
    Material* materialData = nullptr;
    mPMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData->enableLighting = false;
    materialData->uvTransform = MakeIdentity4x4();
    materialData->enableNormalMap = 0;
    mPMaterialResource->Unmap(0, nullptr);


}



void Sprite::Draw(ID3D12GraphicsCommandList* cmdList,
    UINT instanceCount,
    UINT startInstanceLocation
)
{
    cmdList->IASetVertexBuffers(0, 1, &mVertexBufferView);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    cmdList->SetGraphicsRootConstantBufferView(0,
        mPMaterialResource->GetGPUVirtualAddress()
    );

    cmdList->DrawInstanced(4, instanceCount, 0, startInstanceLocation);
}