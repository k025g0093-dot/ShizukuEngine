#pragma once
#include "AllInclude/AllD3D12Include.h"
#include  <algorithm>

using Microsoft::WRL::ComPtr;


ComPtr< ID3D12DescriptorHeap>CreateDescriptorHeap(
	ID3D12Device* device,
	D3D12_DESCRIPTOR_HEAP_TYPE heapType,
	uint32_t numDescriptors,
	bool shaderVisible
);



