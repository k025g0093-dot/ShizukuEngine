#pragma once
#include "AllD3D12Include.h"
#include  <algorithm>


#include "LogSistem.h"
#include "ConvertString.h"


using Microsoft::WRL::ComPtr;

//描画用コマンドを作成したり載せたりいろいろやっちゃうよ
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

private:

	void InitFenceEvent();

	ComPtr < IDXGIAdapter4> useAdapter = nullptr;
	DXGI_ADAPTER_DESC3 adapterDesc = {};//これはコムポでかけない
	ComPtr<ID3D12Device> m_device = nullptr;



	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc {};
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	ComPtr<IDXGISwapChain4> swapChin = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	ComPtr<IDXGIFactory7> dxgiFactory = {};

	ID3D12Resource* swapChainResources[2] = { nullptr };

	ComPtr<ID3D12DescriptorHeap>* rtvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	D3D12_RESOURCE_BARRIER barrier{};
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	HRESULT hr;

	//フェンスイベントの初期化
	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	HANDLE fenceEvent;
};

