#pragma once
#include "AllD3D12Include.h"
#include  <algorithm>


#include "LogSistem.h"
#include "ConvertString.h"


using Microsoft::WRL::ComPtr;

//描画用コマンドを作成したりスワップチェーンなどを作成します
class DX12Context
{

public:

	void InitDXGIFactory(
		HWND hwnd,
		ComPtr<ID3D12Device>* device
	);

	void CreateCommandObjects(
		ComPtr<ID3D12Device> device,
		HWND hwnd,
		int32_t height,int32_t width);

	void PostDraw();
	void PreDraw();



	//コマンドリストのゲッター
	ID3D12GraphicsCommandList *GetCommandList(){ return commandList.Get(); }

private://プライベート関数

	ComPtr< ID3D12DescriptorHeap>CreateDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE heapType,
		uint32_t numDescriptors,
		bool shaderVisible
	);

	void InitFenceEvent();

	ID3D12Resource* CreateDepthStencilTextureResource();

private://メンバ変数

	int32_t mHeight;//スワップチェーンとかを作成する際に使用
	int32_t mWidth;


	ComPtr < IDXGIAdapter4> useAdapter = nullptr;
	DXGI_ADAPTER_DESC3 adapterDesc = {};//これはコムポでかけない
	ComPtr<ID3D12Device> mDevice = nullptr;

	ComPtr<ID3D12Resource> depthStencilResource;


	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc {};
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	ComPtr<IDXGISwapChain4> swapChin = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;


	ComPtr<IDXGIFactory7> dxgiFactory = {};

	ID3D12Resource* swapChainResources[2] = { nullptr };

	//ヒープ関連
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap=nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	D3D12_RESOURCE_BARRIER barrier{};
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	HRESULT hr;

	//フェンスイベントの初期化
	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	HANDLE fenceEvent;
};

