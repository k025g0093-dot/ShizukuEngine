#include "ShizukuEngine.h"

using namespace std;
//インスタンス初期化
ShizukuEngine* ShizukuEngine::instance = nullptr;

#pragma region ウィンドウの初期化

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

//ウィンドウの初期化関数
void ShizukuEngine::InitWindow(
	int Height, int width, wstring WinName
) {

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

#pragma endregion

ShizukuEngine::ShizukuEngine(int Height, int width, wstring WinName){
	InitWindow(Height, width, WinName);
	Initialize();
}

void ShizukuEngine::GetInstance(int Height, int width, wstring WinName) {
	if (instance == nullptr) {
		//インスタンスがない場合はnewをする
		instance = new ShizukuEngine(Height,width,WinName);
	}
}


void ShizukuEngine::Initialize() {

	dxgi.InitDXGIFactory(hwnd);

}

void ShizukuEngine::Run() {}

ShizukuEngine::~ShizukuEngine() {}

void ShizukuEngine::PreDraw() {}

void ShizukuEngine::PostDraw() {}