#pragma once
#include "AllD3D12Include.h"

#include "LogSistem.h"
#include "ConvertString.h"

using Microsoft::WRL::ComPtr;

class DXGI
{
public:
	void InitDXGIFactory(HWND hwnd, ComPtr<ID3D12Device>*device);


private:
	ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	IDXGIAdapter4* useAdapter = nullptr;
	DXGI_ADAPTER_DESC3 adapterDesc = {};//これはコムポでかけない
};

