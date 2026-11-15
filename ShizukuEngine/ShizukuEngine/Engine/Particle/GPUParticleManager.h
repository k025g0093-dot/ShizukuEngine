#pragma once
#include <vector>
#include<memory>
#include "Allinclude/AllD3D12Include.h"
#include "Allinclude/AllVector.h"
#include "Particle/ParticleTypes.h"
#include "Particle/GPUParticleEmitter.h"

using Microsoft::WRL::ComPtr;


class TextureManager;
class Model;

class GPUParticleManager
{

public:

	void Initialize(ID3D12Device* device, TextureManager* textureManager);

	GPUParticleEmitter* GetEmitter(int index)const;

	void Draw(
		ID3D12GraphicsCommandList* commandList,
		const Matrix4x4& viewProjection,
		const Matrix4x4& billboard
	);

private:
	ID3D12Device* mDevice = nullptr;
	TextureManager* mTextureManager = nullptr;

	//CSで使用
	ComPtr<ID3D12RootSignature> mComputeRootSignature;
	ComPtr<ID3D12PipelineState> mComputePipelineState;

	//描画用
	ComPtr<ID3D12RootSignature> mGraphicsRootSignature;
	ComPtr<ID3D12PipelineState> mGraphicsPipelineState;

	ComPtr<ID3D12Resource> mPerViewResource;
	PerView* mPerViewData = nullptr;

	//エミッター
	std::vector<std::unique_ptr<GPUParticleEmitter>> mEmitters;

};

