#pragma once
#include "AllD3D12Include.h"
#include "DXC.h"
#include "LogSistem.h"

using Microsoft::WRL::ComPtr;

enum class PipelineType {
    k3D,
    k2D
};

enum BlendMode {

    //ブレンドなし
    kBlendModeNone,
    //通常αブレンド。デフォルト
    kBlendModeNormal,
    //加算
    kBlendModeAdd,
    //減算
    kBlendModeSubtract,
    //乗算
    kBlendModeMultiply,
    //スクリーン
    kBlendModeScreen,
    //利用しない
    kCountOfBlendMode,
};

ComPtr<ID3D12PipelineState> CreatePipelineStateDesc(
    ID3D12Device* device,
    ComPtr<ID3D12RootSignature>& rootSignature,
    HRESULT& hr,
    PipelineType type,
    BlendMode blendMode
);

ComPtr<ID3D12RootSignature> CreateRootSignature(
    ID3D12Device* device,
    HRESULT& hr);

D3D12_BLEND_DESC CreateBlendState(BlendMode blendMode);
D3D12_RASTERIZER_DESC CreateRasterizerState(PipelineType type);
D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(PipelineType type);
