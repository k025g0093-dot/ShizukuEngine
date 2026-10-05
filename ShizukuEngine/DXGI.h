#pragma once
#include <Windows.h>
#include<wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#include "LogSistem.h"
#include "ConvertString.h"

using Microsoft::WRL::ComPtr;

class DXGI
{
public:
	void InitDXGIFactory(HWND hwnd);


private:
	ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	IDXGIAdapter4* useAdapter = nullptr;
	DXGI_ADAPTER_DESC3 adapterDesc = {};//これはコムポでかけない
	ComPtr<ID3D12Device> device = nullptr;
};

