#pragma once
#include "Allinclude/AllD3D12Include.h"
#include "DXC.h"
#include "Core/LogSistem.h"

using Microsoft::WRL::ComPtr;


ComPtr<ID3D12PipelineState> CreateGPUPipelineStateDesc(
	ID3D12Device* device,
	ComPtr<ID3D12RootSignature>& rootSignature,
	HRESULT& hr);

//ルートシグネチャの作成関数
ComPtr<ID3D12RootSignature> CreateGPURootSignature(
	ID3D12Device* device,
	HRESULT& hr);
