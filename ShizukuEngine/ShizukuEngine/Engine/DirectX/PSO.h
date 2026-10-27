#pragma once
#include "Allinclude/AllD3D12Include.h"
#include "DXC.h"
#include "Core/LogSistem.h"

using Microsoft::WRL::ComPtr;

enum class PipelineType {
    k3D,
    k2D,
    kSkinned3D,
    kParticle,
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

//-----------------------------------------------------
//通常のパイプラインとルートシグネチャの作成関数
//-----------------------------------------------------

//パイプラインステートの作成関数
ComPtr<ID3D12PipelineState> CreatePipelineStateDesc(
    ID3D12Device* device,
    ComPtr<ID3D12RootSignature>& rootSignature,
    HRESULT& hr,
    PipelineType type,
    BlendMode blendMode
);

//ルートシグネチャの作成関数
ComPtr<ID3D12RootSignature> CreateRootSignature(
    ID3D12Device* device,
    HRESULT& hr);


//-----------------------------------------------------
//陰用のパイプラインの作成関数
//-----------------------------------------------------

//陰用のパイプラインステート作成関数
ComPtr<ID3D12PipelineState> CreateShadowPipelineState(
    ID3D12Device* device,
    ComPtr<ID3D12RootSignature>& rootSignature,
    HRESULT& hr
);



D3D12_BLEND_DESC CreateBlendState(BlendMode blendMode);
D3D12_RASTERIZER_DESC CreateRasterizerState(PipelineType type);
D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(PipelineType type);
