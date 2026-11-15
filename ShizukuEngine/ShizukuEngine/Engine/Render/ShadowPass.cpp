#include "ShadowPass.h"

bool ShadowPass::Initialize(
	ID3D12Device* device,
	TextureManager* textureManager,
	ComPtr<ID3D12RootSignature>& rootSig)
{
	//newを行う
	mShadowMapBuffer = new ShadowMapBuffer();
	//初期化をして
	mShadowMapBuffer->Initialize(device, textureManager);
	mShadowPipelineState=
		CreateShadowPipelineState(device, rootSig, mHr,PipelineType::k3D);

	mSkinnedShadowPipelineState=
		CreateShadowPipelineState(device, rootSig, mHr, PipelineType::kSkinned3D);


	mPLightVPResource = CreateBufferResource(device, Align256(sizeof(Matrix4x4)));
	mPLightVPResource->Map(0, nullptr, reinterpret_cast<void**>(&mMappedLightVP));

	return true;
}

void ShadowPass::Begin(ID3D12GraphicsCommandList* cmd,
	ID3D12RootSignature* rootSig) 

{

	mShadowMapBuffer->TransitionToDsv(cmd);
	cmd->SetGraphicsRootSignature(rootSig);
	cmd->SetPipelineState(mShadowPipelineState.Get());


	D3D12_VIEWPORT shadowVP{};
	shadowVP.Width = (float)ShadowMapBuffer::kShadowMapSize;
	shadowVP.Height = (float)ShadowMapBuffer::kShadowMapSize;
	shadowVP.MaxDepth = 1.0f;
	cmd->RSSetViewports(1, &shadowVP);



	D3D12_RECT shadowRect{};
	shadowRect.right = ShadowMapBuffer::kShadowMapSize;
	shadowRect.bottom = ShadowMapBuffer::kShadowMapSize;
	cmd->RSSetScissorRects(1, &shadowRect);

	auto shadowDsv = mShadowMapBuffer->GetDsvCpuHandle();
	cmd->OMSetRenderTargets(0, nullptr, false, &shadowDsv);
	cmd->ClearDepthStencilView(shadowDsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	cmd->SetGraphicsRootConstantBufferView(8, mPLightVPResource->GetGPUVirtualAddress());

}

void ShadowPass::End(ID3D12GraphicsCommandList* cmd) {

	mShadowMapBuffer->TransitionToSrv(cmd);
}

void ShadowPass::Update(const LightData& light,Vector3 shadowCenter)
{
	Vector3 lightDir = light.direction.Normalized();
	Vector3 lightPos = shadowCenter+ lightDir * -50.0f;

	Vector3 up = { 0.0f, 1.0f, 0.0f };
	if (std::abs(lightDir.Dot(up)) > 0.99f) up = { 0.0f, 0.0f, 1.0f };

	Matrix4x4 view = MakeLookAtMatrix(lightPos, shadowCenter, up);
	Matrix4x4 proj = MakeOrthographicMatrix(-15.0f, 15.0f, 15.0f, -15.0f, 0.1f, 100.0f);
	*mMappedLightVP = Multiply(view, proj);
}

void ShadowPass::BindForMainPass(ID3D12GraphicsCommandList* cmd) {
	cmd->SetGraphicsRootDescriptorTable(9, mShadowMapBuffer->GetSrvGpuHandle());
	cmd->SetGraphicsRootConstantBufferView(8, mPLightVPResource->GetGPUVirtualAddress());
}

void ShadowPass::SetShadowPipe(
	ID3D12GraphicsCommandList* cmd,
	bool isSkinned
) {
	if (isSkinned) {
		cmd->SetPipelineState(mSkinnedShadowPipelineState.Get());
	}
	else {
		cmd->SetPipelineState(mShadowPipelineState.Get());
	}
}
