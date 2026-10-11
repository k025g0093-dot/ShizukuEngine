#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "allVector.h"

using Microsoft::WRL::ComPtr;

struct LightData {
    Vector3 dirOrPos;//向きを表すポジション
    int type;//ライトのタイプ
    Vector4 color;//ライトの色
    float intensity;//数
};

class LightManager {
public:

    LightManager();
    ~LightManager();


    static const int maxLight = 10;

    void Initialize(ComPtr<ID3D12Device> device);

    void SetLight(int index, const LightData& light);
    const LightData& GetLight(int index) const { return mLights[index]; }
    int GetActiveLightCount() const { return mActiveLightCount; }

    void Bind(ID3D12GraphicsCommandList* cmdList);

    void SetSelectedLight(int index) { mSelectedLightIndex = index; }
    int GetSelectedLight() const { return mSelectedLightIndex; }

    int AddLight();
    void RemoveLight(int index);
    bool IsLightActive(int index){return mLightActive[index];}


private://プライベート関数
    void Upload();

private://メンバ変数


    ID3D12Device* mDevice = nullptr;

    ComPtr<ID3D12Resource> mLightBuffer;

    LightData mLights[maxLight] = {};
    bool mLightActive[maxLight] = { true };
    int mActiveLightCount = 0;
    int mSelectedLightIndex = -1;
};