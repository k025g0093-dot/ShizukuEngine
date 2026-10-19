#pragma once
#include "ShadowMapBuffer.h"
#include "VertexResource.h"
#include "PSO.h"
#include "allVector.h"
#include "LightManager.h"

using Microsoft::WRL::ComPtr;

class TextureManager;
class ShadowPass
{

public:
	bool Initialize(ID3D12Device* device, TextureManager* textureManager,
		ComPtr<ID3D12RootSignature>& rootSig);

	void Begin(ID3D12GraphicsCommandList* cmd,
		ID3D12RootSignature* rootSig);

	void End(ID3D12GraphicsCommandList* cmd);

	void Update(const LightData& light);

	void BindForMainPass(ID3D12GraphicsCommandList* cmd);

private:

	//shadow.VSに送るためのパイプ設定だったり
	ComPtr<ID3D12PipelineState> mShadowPipelineState;
	ComPtr<ID3D12Resource> mPLightVPResource;

	HRESULT mHr = S_OK;

	ShadowMapBuffer* mShadowMapBuffer = nullptr;

	Matrix4x4* mMappedLightVP = nullptr;

	uint32_t Align256(uint32_t size)
	{
		return (size + 0xff) & ~0xff;
	}
};

