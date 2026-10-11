#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <array>
#include <vector>

#include "externals/DirectXTex/DirectXTex.h"

//テクスチャの読み込みとSRVの管理を行う
class TextureManager
{
public:

	static const int kImGuiReserved = 8;//SRVヒープの先頭でImGui用に空けておく数

	//デバイス、SRVヒープ、コマンドリストは外から借りる
	void Initialize(
		ID3D12Device* device,
		ID3D12DescriptorHeap* srvHeap,
		ID3D12GraphicsCommandList* commandList
	);

	//テクスチャを読み込んで番号を返す（失敗したら-1）
	int LoadTexture(const std::string& filePath);

	//番号からシェーダーに渡すGPUハンドルを取得する
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int index) ;

	//読み込み済みのテクスチャ数
	int GetTextureCount() const { return mTextureCount; }

private://プライベート関数

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTexture(
		ID3D12Resource* texture,
		const DirectX::ScratchImage& mipImages);

	void CreateTextureSRV(
		ID3D12Resource* textureResource,
		const DirectX::TexMetadata& metadata,
		int index);

	//SRVヒープの位置（ImGui分のずらしを含めた番号）からハンドルを計算する
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandleFromHeapIndex(uint32_t heapIndex);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleFromHeapIndex(uint32_t heapIndex) ;

private://メンバ変数

	//外から借りているもの
	ID3D12Device* mDevice = nullptr;
	ID3D12DescriptorHeap* mSrvHeap = nullptr;
	ID3D12GraphicsCommandList* mCommandList = nullptr;

	//読み込んだテクスチャ
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> mTextures;
	int mTextureCount = 0;

	//GPUのコピーが終わるまで保持しておくアップロード用バッファ
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> mUploadResources;

	//SRVヒープの1要素の大きさ
	UINT mDescriptorSize = 0;

	int mTextureCapacity = 0;
};