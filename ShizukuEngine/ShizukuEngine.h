#pragma once
#include "AllInclude.h"

//ComPtr地獄だっきゃくのために必要
using Microsoft::WRL::ComPtr;
using namespace std;

class ShizukuEngine
{
public:
	ShizukuEngine(int Height, int width, wstring WinName);//インスタンス関数
	~ShizukuEngine();//メモリ開放
	void GetInstance(int Height, int width, wstring WinName);//インスタンス取得関数

	//ウィンドウの初期化
	void InitWindow(int32_t Height, int32_t width, wstring WinName);

	void Initialize();
	void Run();
	void PreDraw();
	void PostDraw();

private://ヘルパー関数など内部関数がメイン

	//ヘルパー関数

	DXGI dxgi{};

private://各種変数などの初期化

	static ShizukuEngine* instance;
	HWND hwnd = nullptr;
};

