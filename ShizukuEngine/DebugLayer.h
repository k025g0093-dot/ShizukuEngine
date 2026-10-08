#pragma once
#include <d3d12.h>
#include <assert.h>

class DebugLayer
{
public:
	void EnableDebugLayer();
	void SetupInfoQueue(ID3D12Device* device);
};

