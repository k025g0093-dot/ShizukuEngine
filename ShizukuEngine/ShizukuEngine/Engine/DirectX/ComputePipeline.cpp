#include "ComputePipeline.h"


ComPtr<ID3D12PipelineState> CreateGPUPipelineStateDesc(
	ID3D12Device* device,
	ComPtr<ID3D12RootSignature>& rootSignature,
	HRESULT& hr) 
{

	// DXCコンパイラの初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;
	DxcCompilerInclude(hr, dxcUtils, dxcCompiler, includeHandler);

	D3D12_COMPUTE_PIPELINE_STATE_DESC
		computePipelineStateDesc{};

	IDxcBlob* computerShaderBlob = CompileShader(
		L"shaders/InitializeParticle.CS.hlsl", L"cs_6_0",
		dxcUtils, dxcCompiler, includeHandler);


	computePipelineStateDesc.CS = {
		.pShaderBytecode = computerShaderBlob->GetBufferPointer(),
		.BytecodeLength = computerShaderBlob->GetBufferSize()
	};

	rootSignature = CreateGPURootSignature(device, hr);

	computePipelineStateDesc.pRootSignature = rootSignature.Get();
	ComPtr<ID3D12PipelineState> computePipelineState = nullptr;
	hr = device->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&computePipelineState));
	assert(SUCCEEDED(hr));
	return computePipelineState;

}

ComPtr<ID3D12RootSignature> CreateGPURootSignature(
	ID3D12Device* device,
	HRESULT& hr) 
{

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	D3D12_ROOT_PARAMETER rootParameter[1] = {};

	D3D12_DESCRIPTOR_RANGE descriptorRangeUAV{};
	descriptorRangeUAV.BaseShaderRegister = 0;
	descriptorRangeUAV.NumDescriptors = 1;
	descriptorRangeUAV.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRangeUAV.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[0].DescriptorTable.pDescriptorRanges = &descriptorRangeUAV;
	rootParameter[0].DescriptorTable.NumDescriptorRanges = 1;

	descriptionRootSignature.pParameters = rootParameter;
	descriptionRootSignature.NumParameters = _countof(rootParameter);

	//シリアライズ
	ComPtr < ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

	if (FAILED(hr)) {
		Log(logStream, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	ComPtr<ID3D12RootSignature> rootSignature;
	hr = device->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(rootSignature.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return rootSignature;

}
