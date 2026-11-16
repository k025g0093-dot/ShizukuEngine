#include "RenderTexture.h"
#include "TextureManager/TextureManager.h"

void RenderTexture::CreateSceneRtv(
	ID3D12Device* device,
	TextureManager* textureManager,
	int32_t width,
	int32_t height
) {

	mWidth = width;
	mHeight = height;

	D3D12_RESOURCE_DESC desc{};
	desc.Width = mWidth;
	desc.Height = mHeight;
	desc.MipLevels = 1;
	desc.DepthOrArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
	desc.SampleDesc.Count = 1;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	clearValue.Color[0] = 0.1f;
	clearValue.Color[1] = 0.25f;
	clearValue.Color[2] = 0.5f;
	clearValue.Color[3] = 1.0f;

	mSceneRtvDescriptorHeap.Reset();
	HRESULT hr{};
	hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(mSceneColorResource.ReleaseAndGetAddressOf()));
	assert(SUCCEEDED(hr));

	mSceneRtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, false);
	mSceneRtvHandle = mSceneRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(mSceneColorResource.Get(), &rtvDesc, mSceneRtvHandle);

	mTextureManager = textureManager;

	//SRV作成
	if (mSlot < 0) {
		mSlot = mTextureManager->AllocateSrv();
	}
	assert(mSlot >= 0);

	mSceneSrvCpuHandle = mTextureManager->GetCPUHandle(mSlot);
	mSceneSrvGpuHandle = mTextureManager->GetGPUHandle(mSlot);
	_ASSERT_EXPR(mSceneSrvCpuHandle.ptr != 0, L"SRVのCPUハンドルが取得できませんでした");

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(mSceneColorResource.Get(), &srvDesc, mSceneSrvCpuHandle);
	CreateDepthStencilTextureResource(device, width, height);

}


void RenderTexture::CreateDepthStencilTextureResource(
	ID3D12Device* device,
	int32_t width,
	int32_t height) {

	mWidth = width;
	mHeight = height;

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

	mDepthStencilResource.Reset();
	HRESULT hr{};
	hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(mDepthStencilResource.GetAddressOf())
	);
	assert(SUCCEEDED(hr));

	mDsvDescriptorHeap = CreateDescriptorHeap(
		device,
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		1,
		false
	);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	mDsvHandle = mDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	device->CreateDepthStencilView(
		mDepthStencilResource.Get(),
		&dsvDesc,
		mDsvHandle
	);

}


void RenderTexture::ResizeSceneRenderTarget(
	ID3D12Device* device,
	int newWidth, int newHeight) {

	mWidth = newWidth;
	mHeight = newHeight;

	if (mSceneColorResource) {
		mSceneColorResource.Reset();
	}
	if (mDepthStencilResource) {
		mDepthStencilResource.Reset();
	}
	CreateSceneRtv(device, mTextureManager, mWidth, mHeight);
}

void RenderTexture::SetSceneRenderTarget(
	ID3D12GraphicsCommandList* commandList) {

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
	scissorRect.right = static_cast<LONG>(mWidth);
	scissorRect.top = 0;
	scissorRect.bottom = static_cast<LONG>(mHeight);
	commandList->RSSetScissorRects(1, &scissorRect);

	commandList->OMSetRenderTargets(1, &mSceneRtvHandle, false, &mDsvHandle);
}

void RenderTexture::ClearSceneRenderTarget(
	ID3D12GraphicsCommandList* commandList) {
	const float clearColor[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
	commandList->ClearRenderTargetView(mSceneRtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(mDsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}


void RenderTexture::TransitionToRenderTarget(
	ID3D12GraphicsCommandList* commandList) {

	D3D12_RESOURCE_BARRIER offScreenBarrier{};
	offScreenBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	offScreenBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	offScreenBarrier.Transition.pResource = mSceneColorResource.Get();
	offScreenBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	offScreenBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1, &offScreenBarrier);

}

void RenderTexture::TransitionToSrv(
	ID3D12GraphicsCommandList* commandList)
{
	D3D12_RESOURCE_BARRIER offScreenBarrier{};
	offScreenBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	offScreenBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	offScreenBarrier.Transition.pResource = mSceneColorResource.Get();
	offScreenBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	offScreenBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	commandList->ResourceBarrier(1, &offScreenBarrier);
}