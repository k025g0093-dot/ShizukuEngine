#include "LightManager.h"
#include "DirectX/VertexResource.h"
#include "Core/LogSistem.h"

LightManager::LightManager() {}
LightManager::~LightManager() {}

//ライトの初期化を行います
void LightManager::Initialize(ComPtr<ID3D12Device> device) {
	mDevice = device.Get();

	//ディレクショナルライトの初期化
	LightData defaultLight{};
	defaultLight.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	defaultLight.type = 0;
	defaultLight.direction = { 0.0f, -1.0f, 0.0f };
	defaultLight.intensity = 1.0f;
	mLights[0] = defaultLight;
	mActiveLightCount = 1;

	UINT bufferSize = sizeof(LightData) * maxLight;
	mLightBuffer = CreateBufferResource(mDevice, bufferSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = maxLight;
	srvDesc.Buffer.StructureByteStride = sizeof(LightData);

	LoadLightsFromFile();

	Upload();
}

// LightManager.cpp
void LightManager::SetLight(int index, const LightData& light) {
	if (index < 0 || index >= maxLight) return;
	mLights[index] = light;
	if (index >= mActiveLightCount) mActiveLightCount = index + 1;
	Upload();
}

void LightManager::Bind(ID3D12GraphicsCommandList* cmdList) {
	cmdList->SetGraphicsRootShaderResourceView(
		3, mLightBuffer->GetGPUVirtualAddress());
	cmdList->SetGraphicsRoot32BitConstant(7, mActiveLightCount, 0);
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
int LightManager::AddPointLight() {

	const int index = AllocateLightSlot();
	if (index < 0) { return -1; }

	LightData d{};
	d.type = 1;//ポイントライト
	d.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	d.intensity = 1.0f;
	d.position = { 0.0f, 2.0f, 0.0f };
	d.distance = 20.0f;//届く距離
	mLights[index] = d;
	Upload();
	return index;

}


//スポットライトの追加を行う関数
int LightManager::AddSpotLight() {

	const int index = AllocateLightSlot();
	if (index < 0) { return -1; }

	LightData d{};
	d.type = 2;//スポットライト
	d.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	d.intensity = 1.0f;
	d.position = { 0.0f, 5.0f, 0.0f };
	d.direction = { 0.0f, -1.0f, 0.0f };//真下を照らす
	d.distance = 20.0f;//届く距離
	d.decay = 2.0f;//減衰率
	d.cosAngle = std::cos(std::numbers::pi_v<float> / 6.0f);

	mLights[index] = d;
	Upload();
	return index;
}

int LightManager::AllocateLightSlot() {

	for (int i = 1; i < maxLight; i++) {
		if (!mLightActive[i]) {
			mLightActive[i] = true;
			mActiveLightCount = i + 1;
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

void LightManager::LoadLightsFromFile() {
	std::ifstream file("resources/sceneData/lightsData.json");
	if (file.is_open()) {
	
		json Data = json::parse(file, nullptr, false);
		for (int i = 0; i < Data["lights"].size(); i++) {
		
			mLights[i].direction = { Data["lights"][i]["direction"][0], Data["lights"][i]["direction"][1], Data["lights"][i]["direction"][2] };
			mLights[i].position = { Data["lights"][i]["position"][0], Data["lights"][i]["position"][1], Data["lights"][i]["position"][2] };
			mLights[i].color = { Data["lights"][i]["color"][0], Data["lights"][i]["color"][1], Data["lights"][i]["color"][2], Data["lights"][i]["color"][3] };
			mLights[i].type = Data["lights"][i]["type"];
			mLights[i].intensity = Data["lights"][i]["intensity"];
			mLights[i].distance = Data["lights"][i]["distance"];
			mLights[i].decay = Data["lights"][i]["decay"];
			mLights[i].cosAngle = Data["lights"][i]["cosAngle"];
			mLights[i].cosFalloff = Data["lights"][i]["cosFalloff"];
			mLightActive[i] = true;
			mActiveLightCount = 1 + i;
		}
	
	}
}
void LightManager::SaveLightsToFile() {

	json data;
	data["lights"] = json::array();

	for (int i = 0; i < maxLight; i++) {
		if (!mLightActive[i]) continue;

		json lightData;
		lightData["direction"] = { mLights[i].direction.x, mLights[i].direction.y, mLights[i].direction.z };
		lightData["position"] = { mLights[i].position.x, mLights[i].position.y, mLights[i].position.z };
		lightData["color"] = { mLights[i].color.x, mLights[i].color.y, mLights[i].color.z, mLights[i].color.w };
		lightData["type"] = mLights[i].type;
		lightData["intensity"] = mLights[i].intensity;
		lightData["distance"] = mLights[i].distance;
		lightData["decay"] = mLights[i].decay;
		lightData["cosAngle"] = mLights[i].cosAngle;
		lightData["cosFalloff"] = mLights[i].cosFalloff;

		data["lights"].push_back(lightData);
	}
	std::ofstream file("resources/sceneData/lightsData.json");
	file << data.dump(4);
}
