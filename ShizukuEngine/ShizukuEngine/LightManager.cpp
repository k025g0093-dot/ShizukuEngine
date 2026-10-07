#include "LightManager.h"
#include "VertexResource.h"
#include "LogSistem.h"

//インスタンスの初期化
LightManager* LightManager::mInstance = nullptr;

//インスタンス取得関数
LightManager* LightManager::GetInstance() {
    if (!mInstance) mInstance = new LightManager();
    return mInstance;
}


//ライトの初期化を行います
void LightManager::Initialize(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) {
    mDevice = device;

    //ディレクショナルライトの初期化
    LightData defaultLight{};
    defaultLight.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    defaultLight.type = 0;
    defaultLight.dirOrPos = { 0.0f, -1.0f, 0.0f };
    defaultLight.intensity = 1.0f;
    mLights[0] = defaultLight;
    mActiveLightCount = 1;

    UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    UINT bufferSize = sizeof(LightData) * maxLight;
    mLightBuffer = CreateBufferResource(device, bufferSize);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = maxLight;
    srvDesc.Buffer.StructureByteStride = sizeof(LightData);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    cpuHandle.ptr = srvHeap->GetCPUDescriptorHandleForHeapStart().ptr + descriptorSize * lightSrvSlot;
    device->CreateShaderResourceView(mLightBuffer.Get(), &srvDesc, cpuHandle);

    mLightSrvGpuHandle.ptr = srvHeap->GetGPUDescriptorHandleForHeapStart().ptr + descriptorSize * lightSrvSlot;

    Upload();
}

// LightManager.cpp
void LightManager::SetLight(int index, const LightData& light) {
    if (index < 0 || index >= maxLight) return;
    mLights[index] = light;
    if (index >= mActiveLightCount) mActiveLightCount = index + 1;
    Upload();
}

void LightManager::Bind(ID3D12GraphicsCommandList* cmdList, int id) {
    cmdList->SetGraphicsRootDescriptorTable(3, mLightSrvGpuHandle);
}

void LightManager::Upload() {
    if (!mLightBuffer) return;

    void* p = nullptr;
    HRESULT hr = mLightBuffer->Map(0, nullptr, &p);
    if (SUCCEEDED(hr) && p) {
        memcpy(p, mLights, sizeof(LightData) * maxLight);
        mLightBuffer->Unmap(0, nullptr);
    }
}

//ライトの追加を行う関数
int LightManager::AddLight() {
    for (int i = 1; i < maxLight; i++) {
        if (!mLightActive[i]) {
            mLightActive[i] = true;
            LightData d{};
            d.type = 1;
            d.color = { 1.0f, 1.0f, 1.0f, 1.0f };
            d.intensity = 1.0f;
            d.dirOrPos = { 0.0f, 2.0f, 0.0f };
            mLights[i] = d;
            mActiveLightCount = i + 1;
            Upload();
            return i;
        }
    }
    return -1;
}


void LightManager::RemoveLight(int index) {
    if (index <= 0 || index >= maxLight) return;
    mLightActive[index] = false;
    mLights[index] = LightData{};
    if (mSelectedLightIndex == index) mSelectedLightIndex = -1;

    //  一番後ろの有効indexを再計算
    int lastActive = 0;
    for (int i = 1; i < maxLight; i++) {
        if (mLightActive[i]) lastActive = i;
    }
    mActiveLightCount = lastActive + 1;

    Upload();
}
