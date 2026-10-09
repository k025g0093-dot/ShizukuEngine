#include "Sprite.h"
Sprite::~Sprite() = default;

void Sprite::InitSprite(ComPtr<ID3D12Device> device) {


    // --- 頂点バッファ ---
    mPVertexResource = CreateBufferResource(device.Get(), sizeof(VertexData) * 4);
    mPVertexResource->Map(0, nullptr, reinterpret_cast<void**>(&mPVertexData));

    mVertexBufferView.BufferLocation = mPVertexResource->GetGPUVirtualAddress();
    mVertexBufferView.StrideInBytes = sizeof(VertexData);
    mVertexBufferView.SizeInBytes = sizeof(VertexData) * 4;


    mPVertexResource->Unmap(0, nullptr);

    // --- マテリアルバッファ ---
    mPMaterialResource = CreateBufferResource(device.Get(), Align256(sizeof(Material)));
    Material* materialData = nullptr;
    mPMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData->enableLighting = false;
    materialData->uvTransform = MakeIdentity4x4();
    materialData->enableNormalMap = 0;
    mPMaterialResource->Unmap(0, nullptr);


}



void Sprite::SetUVTransform(const Matrix4x4& uvTransform) {
    if (!mPMaterialResource) return;

    Material* materialData = nullptr;
    mPMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    materialData->uvTransform = uvTransform;
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

    cmdList->SetGraphicsRootConstantBufferView(1,
        mPWvpResource->GetGPUVirtualAddress()
    );
    cmdList->DrawInstanced(4, instanceCount, 0, startInstanceLocation);
}