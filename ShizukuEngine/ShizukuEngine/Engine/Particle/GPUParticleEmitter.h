#pragma once
#include<wrl.h>
#include <cassert>
#include <random>
#include<list>
#include "Allinclude/AllVector.h"
#include "Math/RandomEngine.h"
#include"DeltaTaime/Clock.h"
#include "Math/AABB.h"
#include "DirectX/ComputePipeline.h"
#include "DirectX/VertexResource.h"
#include "ParticleTypes.h"

class TextureManager;
class Model;


class GPUParticleEmitter
{
public:

	GPUParticleEmitter();
	~GPUParticleEmitter();

	void CreateEmitter(
		const Emitter& emitter,
		ComPtr<ID3D12Device> device,
		TextureManager* textureManager,
		int textureIndex, Model*model,
		Vector4 color, bool isFixedColor = false
	);

	void SetAccelerationField(const AccelerationField& field);

	void Update(float timer);

	void Draw(
		ID3D12GraphicsCommandList* commandList,   // コマンドリスト
		ID3D12RootSignature* rootSignature,       // 描画用のルートシグネチャ
		ID3D12PipelineState* pipelineState,       // 描画用のPSO
		D3D12_GPU_VIRTUAL_ADDRESS perViewAddress  // PerViewのCBのアドレス
	);

	const std::list<Particle>& GetParticles() const { return particles; }

	const Emitter& GetEmitter() const { return mEmitter; }

	const AccelerationField& GetField()const { return mAccelerationField; }

	void SetEmitter(Emitter emitter) {
		mEmitter = emitter;
	}

	void SetField(AccelerationField field) {
		mAccelerationField = field;
	}

private://プライベート関数

	Particle MakeNewParticle(std::mt19937& randomEngine, const Emitter& emitter);

private://プライベート変数

	bool mIsFixedColor = false;

	std::list<Particle>particles;
	std::mt19937 mRandomEngine;
	Emitter mEmitter;
	AccelerationField mAccelerationField;

	ComPtr<ID3D12Resource>mParticleBuffer;

	TextureManager* mTextureManager{};
	Model* mModel;

	int mMaxCount= 1024;
	int mUavIndex = -1;
	int mTextureIndex = 0;
};

