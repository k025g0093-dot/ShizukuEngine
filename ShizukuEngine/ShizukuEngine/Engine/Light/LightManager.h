#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "Allinclude/allVector.h"
#include "externals/nlohmann/json.hpp"
#include <numbers>

using json = nlohmann::json;
using Microsoft::WRL::ComPtr;

struct LightData {
    Vector3 direction;//向き
    Vector3 position;//ライトの位置
    Vector4 color;//ライトの色
    int type;//ライトのタイプ
    float intensity;//光度
    float distance;//ライトの届く距離
    float decay;//ライトの減衰率
    float cosAngle;//スポットライトの角度（余弦）
	float cosFalloff;//スポットライトの減衰角度（余弦）
};

class LightManager {
public:

    LightManager();
    ~LightManager();


    static const int maxLight = 100;

    void Initialize(ComPtr<ID3D12Device> device);

    void SetLight(int index, const LightData& light);
    const LightData& GetLight(int index) const { return mLights[index]; }
    int GetActiveLightCount() const { return mActiveLightCount; }

    void Bind(ID3D12GraphicsCommandList* cmdList);

    void SetSelectedLight(int index) { mSelectedLightIndex = index; }
    int GetSelectedLight() const { return mSelectedLightIndex; }

    int AddPointLight();
    int AddSpotLight();

    int AllocateLightSlot();

    void RemoveLight(int index);
    bool IsLightActive(int index){return mLightActive[index];}

    void SaveLightsToFile();

private://プライベート関数
    void Upload();
    void LoadLightsFromFile();

private://メンバ変数


    ID3D12Device* mDevice = nullptr;

    ComPtr<ID3D12Resource> mLightBuffer;

    LightData mLights[maxLight] = {};
    bool mLightActive[maxLight] = { true };
    int mActiveLightCount = 0;
    int mSelectedLightIndex = -1;
};