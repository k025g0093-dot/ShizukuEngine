#include "PSO.h"


#pragma region ルートシグネチャの作成
// ルートシグネチャの生成
// シェーダーにどんなリソースを渡すかを定義する
ComPtr<ID3D12RootSignature> CreateRootSignature(
	ID3D12Device* device,
	HRESULT& hr) {

	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// t0: texture, t1: normal texture, t3: lights
	// t2: InstanceData は RootSRV で直接アドレスを渡すので不要
	D3D12_DESCRIPTOR_RANGE descriptorRange[3] = {};
	descriptorRange[0].BaseShaderRegister = 0;// t0 texture
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRange[1].BaseShaderRegister = 1;// t1 normal texture
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descriptorRange[2].BaseShaderRegister = 4; // t4 shadowMap
	descriptorRange[2].NumDescriptors = 1;
	descriptorRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameter[11] = {};
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[0].Descriptor.ShaderRegister = 0; // b0 material

	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter[1].Descriptor.ShaderRegister = 2; // register(t2)
	rootParameter[1].Descriptor.RegisterSpace = 0;

	rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[2].DescriptorTable.pDescriptorRanges = &descriptorRange[0]; // t0 texture
	rootParameter[2].DescriptorTable.NumDescriptorRanges = 1;

	rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[3].Descriptor.ShaderRegister = 3;

	rootParameter[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[4].DescriptorTable.pDescriptorRanges = &descriptorRange[1]; // t1 normal texture
	rootParameter[4].DescriptorTable.NumDescriptorRanges = 1;

	rootParameter[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameter[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter[5].Constants.ShaderRegister = 1; // b1
	rootParameter[5].Constants.RegisterSpace = 0;
	rootParameter[5].Constants.Num32BitValues = 1;

	// [6] CBV b2 - Camera (pixel)
	rootParameter[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameter[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[6].Constants.ShaderRegister = 2;
	rootParameter[6].Constants.Num32BitValues = 3;


	// [7] 32bit constant b3 - ActiveLightCount (pixel)
	rootParameter[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameter[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[7].Constants.ShaderRegister = 3; // b3
	rootParameter[7].Constants.RegisterSpace = 0;
	rootParameter[7].Constants.Num32BitValues = 1; // uintひとつだけ送る

	// [8] CBV b4 - ライトVP (ALL visibility)
	rootParameter[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[8].Descriptor.ShaderRegister = 4;

	// [9] DescriptorTable t4 - シャドウマップ
	rootParameter[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[9].DescriptorTable.pDescriptorRanges = &descriptorRange[2];
	rootParameter[9].DescriptorTable.NumDescriptorRanges = 1;

	rootParameter[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter[10].Descriptor.ShaderRegister = 5;

	descriptionRootSignature.pParameters = rootParameter;
	descriptionRootSignature.NumParameters = _countof(rootParameter);


	//Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[2] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0~1の範囲を繰り返す
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//最大LOD
	staticSamplers[0].ShaderRegister = 0;//register(s0)に対応
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダーで使用

	staticSamplers[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;//バイリニアフィルタ
	staticSamplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;//0~1の範囲を繰り返す
	staticSamplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	staticSamplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	staticSamplers[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	staticSamplers[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;//比較しない
	staticSamplers[1].MaxLOD = D3D12_FLOAT32_MAX;//最大LOD
	staticSamplers[1].ShaderRegister = 1;//register(s1)に対応
	staticSamplers[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダーで使用


	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);



	// ルートシグネチャをバイナリに変換
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	// L"Object3d.VS.hlsl" を L"VertexShader.hlsl" に変更
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

	if (FAILED(hr)) {
		Log(logStream, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// ルートシグネチャの生成
	ComPtr<ID3D12RootSignature> rootSignature;
	hr = device->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(rootSignature.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return rootSignature;
}

// インプットレイアウトの生成
// 頂点データの形式をGPUに教える
D3D12_INPUT_LAYOUT_DESC CreateLayout() {

	// 頂点データの要素定義（今回はPOSITIONのみ）
	static D3D12_INPUT_ELEMENT_DESC inputElementDescs[4] = {};
	inputElementDescs[0].SemanticName = "POSITION"; // シェーダー側のセマンティクス名
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // float4形式
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD"; // シェーダー側のセマンティクス名
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT; // float2形式
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset =
		D3D12_APPEND_ALIGNED_ELEMENT;


	inputElementDescs[3].SemanticName = "TANGENT";
	inputElementDescs[3].SemanticIndex = 0;
	inputElementDescs[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
	return inputLayoutDesc;
}


// スキンモデル用のインプットレイアウト
// SkinnedModel.h の Vertex と同じ並びにする
D3D12_INPUT_LAYOUT_DESC CreateSkinnedLayout() {

	// 関数を抜けても残るようにstaticにする
	static D3D12_INPUT_ELEMENT_DESC inputElementDescs[7] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT; // glm::vec3
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "NORMAL";
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "TEXCOORD";
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32_FLOAT; // glm::vec2
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[3].SemanticName = "TANGENT";
	inputElementDescs[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[4].SemanticName = "BINORMAL";
	inputElementDescs[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[4].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[5].SemanticName = "BLENDINDICES";
	inputElementDescs[5].Format = DXGI_FORMAT_R32G32B32A32_SINT; // int[4]
	inputElementDescs[5].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[6].SemanticName = "BLENDWEIGHT";
	inputElementDescs[6].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // float[4]
	inputElementDescs[6].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
	return inputLayoutDesc;
}


#pragma endregion

// ブレンドステートの生成
// 色の合成方法を定義する
// ブレンドステートの生成
// ブレンドモードに合わせて、シェーダーの出力色(Src)と画面の色(Dest)の混ぜ方を決める
D3D12_BLEND_DESC CreateBlendState(BlendMode blendMode) {
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL; // 全チャンネルへの書き込みを有効化

	// ブレンドなしなら、ここで終わり
	if (blendMode == kBlendModeNone) {
		blendDesc.RenderTarget[0].BlendEnable = FALSE;
		return blendDesc;
	}

	blendDesc.RenderTarget[0].BlendEnable = TRUE;

	// α：全モード共通（出力のαをそのまま使う）
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	// 色：モードごとに切り替える
	switch (blendMode) {
	case kBlendModeNormal:
		// Src × α + Dest × (1 - α)
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		break;
	case kBlendModeAdd:
		// Src × α + Dest × 1
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	case kBlendModeSubtract:
		// Dest × 1 - Src × α
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	case kBlendModeMultiply:
		// Src × 0 + Dest × Src
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
		break;
	case kBlendModeScreen:
		// Src × (1 - Dest) + Dest × 1
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		break;
	default:
		// 想定外の値（kCountOfBlendModeなど）はNormalと同じにしておく
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		break;
	}
	return blendDesc;
}

// ラスタライザステートの生成
// 3Dは裏面を描画しない、2Dは裏返しても消えないようにカリングなし
D3D12_RASTERIZER_DESC CreateRasterizerState(PipelineType type) {
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode =
		(type == PipelineType::k2D) ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID; // ポリゴンを塗りつぶして描画
	return rasterizerDesc;
}

D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(PipelineType type) {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	if (type == PipelineType::k3D||type==PipelineType::kSkinned3D) {
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	}
	else {
		depthStencilDesc.DepthEnable = false;
	}
	return depthStencilDesc;
}

// パイプラインステートオブジェクト（PSO）の生成
// 描画に必要な全設定をまとめたオブジェクトを作る
ComPtr<ID3D12PipelineState> CreatePipelineStateDesc(
	ID3D12Device* device,
	ComPtr<ID3D12RootSignature>& rootSignature,
	HRESULT& hr,
	PipelineType type,
	BlendMode blendMode)
{
	// DXCコンパイラの初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;
	IDxcBlob* vertexShaderBlob = nullptr;
	DxcCompilerInclude(hr, dxcUtils, dxcCompiler, includeHandler);

	// シェーダーのコンパイル
	if (type == PipelineType::kSkinned3D) {

		vertexShaderBlob = CompileShader(
			L"shaders/SkinnedObject3d.VS.hlsl", L"vs_6_0",
			dxcUtils, dxcCompiler, includeHandler);
	}
	else{
		vertexShaderBlob = CompileShader(
			L"shaders/Object3d.VS.hlsl", L"vs_6_0",
			dxcUtils, dxcCompiler, includeHandler);
	}
	IDxcBlob* pixelShaderBlob = CompileShader(
		L"shaders/Object3d.PS.hlsl", L"ps_6_0",
		dxcUtils, dxcCompiler, includeHandler);

	// 各設定の生成（入力レイアウトは3D・2D共通、残り3つは種類で切り替え）
	D3D12_INPUT_LAYOUT_DESC inputLayout = 
		(type == PipelineType::kSkinned3D) ? CreateSkinnedLayout() : CreateLayout();
	D3D12_BLEND_DESC blendDesc = CreateBlendState(blendMode);
	D3D12_RASTERIZER_DESC rasterizerDesc = CreateRasterizerState(type);
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = CreateDepthStencilState(type);

	// PSOの設定をまとめる
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};

	// ルートシグネチャの設定（作成済みのものを使う）
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();

	// インプットレイアウトの設定
	graphicsPipelineStateDesc.InputLayout = inputLayout;

	// シェーダーの設定
	graphicsPipelineStateDesc.VS = {
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize()
	};
	graphicsPipelineStateDesc.PS = {
		pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize()
	};

	// ブレンド・ラスタライザ・深度の設定
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// レンダーターゲットの設定
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// プリミティブトポロジーの設定（STRIPも三角形の仲間なのでTRIANGLEのまま）
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// サンプリングの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// PSOの生成
	ComPtr<ID3D12PipelineState> graphicsPipelineState;
	hr = device->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc,
		IID_PPV_ARGS(graphicsPipelineState.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return graphicsPipelineState;
}


ComPtr<ID3D12PipelineState> CreateShadowPipelineState(
	ID3D12Device* device,
	ComPtr<ID3D12RootSignature>& rootSignature,
	HRESULT& hr
) {
	// DXCコンパイラの初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;
	DxcCompilerInclude(hr, dxcUtils, dxcCompiler, includeHandler);

	// 頂点シェーダーのみコンパイル（深度だけでいいのでピクセルシェーダーは不要）
	IDxcBlob* vertexShaderBlob = CompileShader(
		L"shaders/shadow.VS.hlsl", L"vs_6_0",
		dxcUtils, dxcCompiler, includeHandler);

	// 各設定の生成（既存のヘルパーを流用）
	D3D12_INPUT_LAYOUT_DESC inputLayout = CreateLayout();
	D3D12_BLEND_DESC blendDesc = CreateBlendState(kBlendModeNone);
	D3D12_RASTERIZER_DESC rasterizerDesc = CreateRasterizerState(PipelineType::k3D);

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	// PSOの設定をまとめる（グラフィックス用）
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};

	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();

	graphicsPipelineStateDesc.InputLayout = inputLayout;

	graphicsPipelineStateDesc.VS = {
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize()
	};
	// ピクセルシェーダーは設定しない（深度専用、色出力なし）
	graphicsPipelineStateDesc.PS = {}; // 空のまま

	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT; // ShadowMapBufferと合わせる

	graphicsPipelineStateDesc.NumRenderTargets = 0; //  カラー出力は無し
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	ComPtr<ID3D12PipelineState> graphicsPipelineState;
	hr = device->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc,
		IID_PPV_ARGS(graphicsPipelineState.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return graphicsPipelineState;
}
