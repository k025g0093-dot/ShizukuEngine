#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <array>
#include <cstdint>

struct ImGui_ImplDX12_InitInfo;

//ImGuiの初期化、フレームの開始、描画、終了処理をまとめて行う
//SRVヒープの先頭にある予約枠（TextureManager::kImGuiReserved）をImGuiのテクスチャ用に使う
class ImGuiManager
{
public:
	ImGuiManager() = default;
	~ImGuiManager();

	//ImGuiのコンテキストは1つだけなのでコピーさせない
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;

	//デバイス、コマンドキュー、SRVヒープは外から借りる
	void Initialize(
		HWND hwnd,
		ID3D12Device* device,
		ID3D12CommandQueue* commandQueue,
		ID3D12DescriptorHeap* srvHeap,
		uint32_t reservedCount,
		DXGI_FORMAT rtvFormat,
		DXGI_FORMAT dsvFormat
	);

	//フレームの開始（ImGuiの関数はこれより後で呼ぶ）
	void Begin();

	//ImGuiの描画コマンドを積む（3D/2Dを描いた後、PRESENTへのバリアの前に呼ぶ）
	void End(ID3D12GraphicsCommandList* commandList);

	void RenderPlatformWindows();

private://プライベート関数

	//ImGuiがテクスチャ用のSRVを必要としたときに呼ばれる
	static void AllocateSrv(
		ImGui_ImplDX12_InitInfo* info,
		D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle,
		D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle
	);

	//ImGuiがテクスチャを捨てたときに呼ばれる
	static void FreeSrv(
		ImGui_ImplDX12_InitInfo* info,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle
	);

private://メンバ変数

	//予約枠の最大数（TextureManager::kImGuiReservedより大きくしておく）
	static constexpr uint32_t kMaxSrvCount = 8;

	//外から借りているもの
	ID3D12DescriptorHeap* mSrvHeap = nullptr;

	//SRVヒープの1要素の大きさ
	UINT mDescriptorSize = 0;

	//予約枠のうち実際に使う数と、それぞれが使用中かどうか
	uint32_t mReservedCount = 0;
	std::array<bool, kMaxSrvCount> mIsSrvUsed{};

	bool mIsInitialized = false;
};
