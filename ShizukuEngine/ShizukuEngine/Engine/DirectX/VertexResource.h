#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <wrl.h>
#include "Core/LogSistem.h"
#include "DXC.h"
#include "Core/ConvertString.h"
#include <format>
#include <string>
#include "Math/Vector4.h"

using Microsoft::WRL::ComPtr;

ComPtr<ID3D12Resource> CreateVertexResource(
    ID3D12Device* device,
    size_t sizeInBytes,
    HRESULT& hr);


D3D12_VERTEX_BUFFER_VIEW CreateVertexBufferView(
    ID3D12Resource* vertexResource,
    size_t sizeInBytes,
    size_t strideInBytes);

ComPtr<ID3D12Resource> CreateBufferResource(
    ID3D12Device* device,
    size_t sizeInBytes,
    D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_UPLOAD,
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE
);