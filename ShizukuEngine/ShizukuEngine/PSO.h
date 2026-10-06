#pragma once
#include "AllD3D12Include.h"
#include "DXC.h"
#include "LogSistem.h"

using Microsoft::WRL::ComPtr;

ComPtr<ID3D12PipelineState> CreatePipelineStateDesc(
    ID3D12Device* device,
    ComPtr<ID3D12RootSignature>& rootSignature,
    HRESULT& hr);

ComPtr<ID3D12RootSignature> CreateRootSignature(
    ID3D12Device* device,
    HRESULT& hr);

D3D12_INPUT_LAYOUT_DESC CreateLayout();
D3D12_BLEND_DESC CreateBlendState();
D3D12_RASTERIZER_DESC CreateRasterizerState();
