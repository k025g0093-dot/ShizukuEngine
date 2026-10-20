#pragma once
#include <d3d12.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;
class TextureManager;

class ShadowMapBuffer
{
public:
    static constexpr UINT kShadowMapSize = 1024;
    static ShadowMapBuffer* GetInstance();

    bool Initialize(ID3D12Device* device, TextureManager* textureManager);
    void Bind(ID3D12GraphicsCommandList* cmdList, int rootParamIndex);
    // ShadowMapBuffer.h に追加
    void TransitionToSrv(ID3D12GraphicsCommandList* cmdList);
    void TransitionToDsv(ID3D12GraphicsCommandList* cmdList);

    ID3D12Resource* GetTexture() const { return mPShadowMapResource.Get(); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle() const { return mSrvGpuHandle; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDsvCpuHandle() const { return mDsvHeap->GetCPUDescriptorHandleForHeapStart(); }
private:
    static ShadowMapBuffer* mInstance;

    ComPtr<ID3D12DescriptorHeap>  mDsvHeap;          // DSV用ヒープ
    ComPtr<ID3D12Resource>        mPShadowMapResource; // テクスチャ本体
    D3D12_GPU_DESCRIPTOR_HANDLE mSrvGpuHandle{};
    UINT mSrvHeapSlot = 0;
};
