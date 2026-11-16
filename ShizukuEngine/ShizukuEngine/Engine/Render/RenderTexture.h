#pragma once
#include "DirectX/PSO.h"
#include "AllInclude/AllD3D12Include.h"
#include "../DirectX/DescriptorHeap.h"
#include <algorithm> 
#include<array>

using Microsoft::WRL::ComPtr;

class TextureManager;
class DX12Context;

class RenderTexture
{

public:

	RenderTexture() = default;
	~RenderTexture() = default;


	void CreateSceneRtv(
		ID3D12Device* device, 
		TextureManager* textureManager,
		int32_t width, int32_t height);

	void ResizeSceneRenderTarget(
		ID3D12Device* device,
		int newWidth, int newHeight);

	void  SetSceneRenderTarget(
		ID3D12GraphicsCommandList* commandList);

	void ClearSceneRenderTarget(
		ID3D12GraphicsCommandList* commandList);

	void TransitionToRenderTarget(
		ID3D12GraphicsCommandList* commandList);

	void TransitionToSrv(
		ID3D12GraphicsCommandList* commandList);

	D3D12_GPU_DESCRIPTOR_HANDLE GetSceneSrvGpuHandle() const { return mSceneSrvGpuHandle; }

private://プライベート関数

	void CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);

private://プライベート変数

	int mWidth = 0;
	int mHeight = 0;

	ComPtr<ID3D12DescriptorHeap> mSceneRtvDescriptorHeap;
	ComPtr<ID3D12Resource> mSceneColorResource;
	ComPtr<ID3D12Resource> mDepthStencilResource;
	ComPtr<ID3D12DescriptorHeap> mDsvDescriptorHeap;

	TextureManager* mTextureManager = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE mDsvHandle{};
	D3D12_CPU_DESCRIPTOR_HANDLE mSceneRtvHandle{};
	D3D12_CPU_DESCRIPTOR_HANDLE mSceneSrvCpuHandle{};
	D3D12_GPU_DESCRIPTOR_HANDLE mSceneSrvGpuHandle{};

	int mSlot = -1;

};

