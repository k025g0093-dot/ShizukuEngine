#pragma once
#include "AllD3D12Include.h"
#include  <algorithm>

using Microsoft::WRL::ComPtr;

//描画用コマンドを作成したり載せたりいろいろやっちゃうよ
class DX12Context
{

public:

	void CreateCommandObjects(
		ComPtr<ID3D12Device> device,
		ComPtr<IDXGIFactory7> dxgiFactory,
		HWND hwnd,
		int32_t height,int32_t width);

private:

	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc {};
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	ComPtr<IDXGISwapChain4> swapChin = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	ComPtr<ID3D12DescriptorHeap>* rtvDescriptorHeap = nullptr;
	ComPtr<D3D12_DESCRIPTOR_HEAP_DESC> rtvDescriptorHeapDesc{};

};

