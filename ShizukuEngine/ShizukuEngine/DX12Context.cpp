#include "DX12Context.h"


void DX12Context::InitDXGIFactory(HWND hwnd, ComPtr<ID3D12Device>* device)
{


	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));

	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter))
		!= DXGI_ERROR_NOT_FOUND; i++
		)
	{
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			Log(logStream, ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}
	assert(useAdapter != nullptr);

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelString[] = { "12.2","12.1","12.0" };

	for (size_t i = 0; i < _countof(featureLevels); i++) {
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(device->GetAddressOf()));

		if (SUCCEEDED(hr)) {

			Log(logStream, std::format("Feature Level {} is supported.\n", featureLevelString[i]));
			break;
		}

	}

	assert(device != nullptr);
	Log(logStream, "Complete DirectX 12 Device Creation.\n");


}


void DX12Context::CreateCommandObjects(
	ComPtr<ID3D12Device> device,
	HWND hwnd,
	int32_t height, int32_t width) {

	mDevice = device;

	hr = mDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));

	//コマンドアロケータを生成
	hr = mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(hr));

	//コマンドリストの生成
	hr = mDevice->CreateCommandList(
		0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator.Get(), nullptr,
		IID_PPV_ARGS(commandList.GetAddressOf())
	);

	assert(SUCCEEDED(hr));

	//スワップチェーン作成
	mHeight = height;
	mWidth = width;

	CreateDepthStencilTextureResource();

	swapChainDesc.Width = mWidth;
	swapChainDesc.Height = mHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	hr = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(), hwnd, &swapChainDesc,
		nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChin.GetAddressOf())
	);
	assert(SUCCEEDED(hr));

	rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

	srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);


	hr = (swapChin->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0])));
	//うまく取得できない場合は起動しない
	assert(SUCCEEDED(hr));
	hr = (swapChin->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1])));
	assert(SUCCEEDED(hr));


	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//２Dテクスチャ
	//ディスクリプタの先頭取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//RTVを二個つくる
	rtvHandles[0] = rtvStartHandle;
	mDevice->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);
	rtvHandles[1].ptr = rtvHandles[0].ptr + mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDevice->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);

	InitFenceEvent();


}

//描画前のコマンドを集積
void DX12Context::PreDraw()
{

	UINT backBufferIndex = swapChin->GetCurrentBackBufferIndex();

	//バリアの作成
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = swapChainResources[backBufferIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1, &barrier);


	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//ウィンドウの元の色
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);


	//descriptorHeapsの作成
	ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { srvDescriptorHeap };
	commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());//そしてセット


	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle =
		dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	D3D12_VIEWPORT viewport{};
	viewport.Width = static_cast<float>(mWidth);
	viewport.Height = static_cast<float>(mHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	commandList->RSSetViewports(1, &viewport);

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = mWidth;
	scissorRect.top = 0;
	scissorRect.bottom = mHeight;
	commandList->RSSetScissorRects(1, &scissorRect);

}



void DX12Context::PostDraw()
{

	ID3D12CommandList* commandLists[] = { commandList.Get() };

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &barrier);

	hr = commandList->Close();
	assert(SUCCEEDED(hr));


	commandQueue->ExecuteCommandLists(1, commandLists);
	swapChin->Present(1, 0);

	fenceValue++;

	commandQueue->Signal(fence, fenceValue);
	if (fence->GetCompletedValue() < fenceValue) {
		fence->SetEventOnCompletion(fenceValue, fenceEvent);

		WaitForSingleObject(fenceEvent, INFINITE);
	}

	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));



}


//フェンスイベントの初期化をします
void DX12Context::InitFenceEvent() {
	fence = nullptr;
	fenceValue = 0;
	hr = mDevice->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);

	fenceValue++;

	commandQueue->Signal(fence, fenceValue);
	if (fence->GetCompletedValue() < fenceValue) {
		fence->SetEventOnCompletion(fenceValue, fenceEvent);

		WaitForSingleObject(fenceEvent, INFINITE);
	}
}


//ディスクリプタヒープの作成今後使用するために今のうちに作成
ComPtr<ID3D12DescriptorHeap> DX12Context::CreateDescriptorHeap(
	D3D12_DESCRIPTOR_HEAP_TYPE heapType,
	uint32_t numDescriptors,
	bool shaderVisible
) {

	ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = mDevice->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(descriptorHeap.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return descriptorHeap;
}

ID3D12Resource* DX12Context::CreateDepthStencilTextureResource() {

	D3D12_RESOURCE_DESC depthResourceDesc{};

	depthResourceDesc.Width = mWidth;
	depthResourceDesc.Height = mHeight;
	depthResourceDesc.MipLevels = 1;
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	depthStencilResource.Reset();
	hr = mDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(depthStencilResource.GetAddressOf())
	);
	assert(SUCCEEDED(hr));

	dsvDescriptorHeap = CreateDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		1,
		false
	);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	mDevice->CreateDepthStencilView(
		depthStencilResource.Get(),
		&dsvDesc,
		dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
	);

	return depthStencilResource.Get();

}
