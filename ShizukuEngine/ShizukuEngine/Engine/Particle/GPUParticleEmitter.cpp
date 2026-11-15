#include"GPUParticleEmitter.h"
#include "TextureManager/TextureManager.h"
#include "Model/Model.h"
GPUParticleEmitter::GPUParticleEmitter() {
	std::random_device randomDevice{};
	mRandomEngine.seed(randomDevice());
}

GPUParticleEmitter::~GPUParticleEmitter() {}

Particle GPUParticleEmitter::MakeNewParticle(
	std::mt19937& randomEngine,
	const Emitter& emitter) {

	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);

	Particle particle;
	particle.scale = { 1.0f,1.0f,1.0f };
	particle.rotate = { 0.0f,0.0f,0.0f };
	particle.position = {
		emitter.position.x + distribution(randomEngine),
		emitter.position.y + distribution(randomEngine) ,
		emitter.position.z + distribution(randomEngine)
	};

	particle.velocity = {
		distribution(randomEngine) * emitter.velocityRange.x,
		distribution(randomEngine) * emitter.velocityRange.y ,
		distribution(randomEngine) * emitter.velocityRange.z
	};

	if (mIsFixedColor) {
		particle.color = { 1.0f,1.0f,1.0f,1.0f };
	}
	else {
		particle.color = {
			distColor(randomEngine) * emitter.color.r,
			distColor(randomEngine) * emitter.color.g,
			distColor(randomEngine) * emitter.color.b,
			1.0f
		};
	}

	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;

	return particle;
}

void GPUParticleEmitter::Update(float timer) {

	//----- 発生 -----

	mEmitter.frequencyTime += timer;
	if (mEmitter.frequency <= mEmitter.frequencyTime) {

		for (uint32_t count = 0; count < mEmitter.count; ++count) {
			particles.push_back(
				MakeNewParticle(mRandomEngine, mEmitter)
			);
		}
		mEmitter.frequencyTime -= mEmitter.frequency;
	}

	//----- 更新 -----
	for (std::list<Particle>::iterator particleIterator = particles.begin();
		particleIterator != particles.end();
		) {
		//寿命が尽きたパーティクルは更新しない
		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			particleIterator = particles.erase(particleIterator);
			continue;
		}

		if (IsAABBToPointCollision(mAccelerationField.area, particleIterator->position)) {
			particleIterator->velocity += mAccelerationField.acceleration * timer;
		}

		particleIterator->position += particleIterator->velocity * timer;

		particleIterator->currentTime += timer;

		particleIterator->color.w = 1.0f - (particleIterator->currentTime / particleIterator->lifeTime);
		++particleIterator;
	}
}

void GPUParticleEmitter::CreateEmitter(
	const Emitter& emitter,
	ComPtr<ID3D12Device> device,
	TextureManager* textureManager,
	int textureIndex, Model* model,
	Vector4 color, bool isFixedColor
) {
	mEmitter = emitter;
	mEmitter.color = color;
	mIsFixedColor = isFixedColor;
	mTextureIndex = textureIndex;
	mModel = model;

	for (uint32_t count = 0; count < mEmitter.count; ++count) {
		particles.push_back(
			MakeNewParticle(mRandomEngine, mEmitter)
		);
	}

	mParticleBuffer = CreateBufferResource(
		device.Get(),
		sizeof(Particle) * mMaxCount,
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);

	mTextureManager = textureManager;

	mUavIndex = textureManager->AllocateSrv();
	assert(mUavIndex != -1 && "SRVヒープに空きがない");

	//UAVの設定（作ったら使わないのでローカル）
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = mMaxCount;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.StructureByteStride = sizeof(Particle);

	device->CreateUnorderedAccessView(
		mParticleBuffer.Get(),
		nullptr,
		&uavDesc,
		textureManager->GetCPUHandle(mUavIndex));

}

void GPUParticleEmitter::SetAccelerationField(const AccelerationField& field) {

	mAccelerationField = field;
	mAccelerationField.acceleration = { field.acceleration.x,field.acceleration.y,field.acceleration.z };
	mAccelerationField.area.min = { field.area.min.x, field.area.min.y, field.area.min.z };
	mAccelerationField.area.max = { field.area.max.x, field.area.max.y, field.area.max.z };
}

void GPUParticleEmitter::Draw(
	ID3D12GraphicsCommandList* commandList,
	ID3D12RootSignature* rootSignature,
	ID3D12PipelineState* pipelineState,
	D3D12_GPU_VIRTUAL_ADDRESS perViewAddress
)
{
	if (!mModel) { return; }
	commandList->SetComputeRootSignature(rootSignature);
	commandList->SetPipelineState(pipelineState);

	//粒のバッファー
	commandList->SetGraphicsRootShaderResourceView(1, mParticleBuffer->GetGPUVirtualAddress());
	//テクスチャの設定
	commandList->SetGraphicsRootDescriptorTable(2, mTextureManager->GetGPUHandle(mTextureIndex));
	//PreViewの設定
	commandList->SetGraphicsRootConstantBufferView(3, perViewAddress);

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = mParticleBuffer.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	commandList->ResourceBarrier(1, &barrier);

	commandList->Dispatch(1, 1, 1);

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

	mModel->Draw(commandList, mMaxCount, 0);

}
