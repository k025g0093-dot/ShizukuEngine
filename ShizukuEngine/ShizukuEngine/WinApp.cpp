#include "WinApp.h"

WinApp::WinApp() {};
WinApp::~WinApp() {};

LRESULT CALLBACK WindowProc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
) {
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::InitWindow(int Height, int width, std::wstring WinName) {
	//ウィンドウクラスの登録
	WNDCLASS ws{};
	ws.lpfnWndProc = WindowProc;
	ws.lpszClassName = L"ShizukuEngine";
	ws.hInstance = GetModuleHandle(nullptr);
	ws.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&ws);

	//ウィンドウサイズの定義

	RECT wrc = { 0,0,width,Height };

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	hwnd = CreateWindow
	(
		ws.lpszClassName,
		WinName.c_str(),
		WS_OVERLAPPEDWINDOW,//よく見るwindowスタイル
		CW_USEDEFAULT,//表示座標X
		CW_USEDEFAULT,//Y
		wrc.right - wrc.left,//window横幅
		wrc.bottom - wrc.top,//windowの縦幅
		nullptr,//親windowハンドル
		nullptr,//メニューハンドル
		ws.hInstance,//インスタンスハンドル
		nullptr//オプション
	);

	ShowWindow(hwnd, SW_SHOW);

	assert(hwnd != nullptr); // 作成に失敗していないか確認する
}
