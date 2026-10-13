#include "ImGuiManager.h"
#include <cassert>
#include <filesystem>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

ImGuiManager::~ImGuiManager() {
	if (!mIsInitialized) { return; }
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiManager::Initialize(
	HWND hwnd,
	ID3D12Device* device,
	ID3D12CommandQueue* commandQueue,
	ID3D12DescriptorHeap* srvHeap,
	uint32_t reservedCount,
	DXGI_FORMAT rtvFormat,
	DXGI_FORMAT dsvFormat
) {
	assert(reservedCount <= kMaxSrvCount);
	mSrvHeap = srvHeap;
	mReservedCount = reservedCount;
	mDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	//標準フォントには日本語が無いので、Windowsのメイリオがあれば使う
	const char* kJapaneseFontPath = "C:/Windows/Fonts/meiryo.ttc";
	if (std::filesystem::exists(kJapaneseFontPath)) {
		ImGui::GetIO().Fonts->AddFontFromFileTTF(kJapaneseFontPath, 18.0f);
	}

	ImGui_ImplWin32_Init(hwnd);

	ImGui_ImplDX12_InitInfo initInfo{};
	initInfo.Device = device;
	initInfo.CommandQueue = commandQueue;
	initInfo.NumFramesInFlight = 2;
	initInfo.RTVFormat = rtvFormat;
	initInfo.DSVFormat = dsvFormat;
	initInfo.UserData = this;
	initInfo.SrvDescriptorHeap = srvHeap;
	initInfo.SrvDescriptorAllocFn = &ImGuiManager::AllocateSrv;
	initInfo.SrvDescriptorFreeFn = &ImGuiManager::FreeSrv;

	bool isSucceeded = ImGui_ImplDX12_Init(&initInfo);
	assert(isSucceeded);
	(void)isSucceeded;//Releaseでassertが消えたときの未使用警告よけ

	mIsInitialized = true;
}

void ImGuiManager::Begin() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::End(ID3D12GraphicsCommandList* commandList) {
	ImGui::Render();

	//ImGuiのテクスチャも同じSRVヒープにあるので、念のためセットし直す
	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvHeap };
	commandList->SetDescriptorHeaps(1, descriptorHeaps);

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void ImGuiManager::AllocateSrv(
	ImGui_ImplDX12_InitInfo* info,
	D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle,
	D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle
) {
	ImGuiManager* self = static_cast<ImGuiManager*>(info->UserData);

	//空いている予約枠を先頭から探す
	for (uint32_t i = 0; i < self->mReservedCount; i++) {
		if (self->mIsSrvUsed[i]) { continue; }
		self->mIsSrvUsed[i] = true;

		outCpuHandle->ptr = self->mSrvHeap->GetCPUDescriptorHandleForHeapStart().ptr + static_cast<SIZE_T>(i) * self->mDescriptorSize;
		outGpuHandle->ptr = self->mSrvHeap->GetGPUDescriptorHandleForHeapStart().ptr + static_cast<UINT64>(i) * self->mDescriptorSize;
		return;
	}

	//予約枠が足りない（kImGuiReservedを増やす必要がある）
	assert(false && "ImGui用のSRVの予約枠が足りません");
}

void ImGuiManager::FreeSrv(
	ImGui_ImplDX12_InitInfo* info,
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
	D3D12_GPU_DESCRIPTOR_HANDLE /*gpuHandle*/
) {
	ImGuiManager* self = static_cast<ImGuiManager*>(info->UserData);

	//ヒープ先頭からの位置で何番目の枠かを求める
	SIZE_T offset = cpuHandle.ptr - self->mSrvHeap->GetCPUDescriptorHandleForHeapStart().ptr;
	uint32_t index = static_cast<uint32_t>(offset / self->mDescriptorSize);
	assert(index < self->mReservedCount);
	self->mIsSrvUsed[index] = false;
}
