#pragma once
#include <d3d12.h>
#include <assert.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

class DebugLayer
{
public:
	void EnableDebugLayer();
	void SetupInfoQueue(ComPtr<ID3D12Device> device);
};

