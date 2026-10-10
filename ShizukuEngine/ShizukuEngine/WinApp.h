#pragma once
#include <string>
#include "AllD3D12Include.h"



class WinApp
{

public:
	WinApp();
	~WinApp();
	void InitWindow(int width, int Height,  std::wstring WinName);

	const HWND GetHwnd() const { return hwnd; }

private://メンバ変数
	HWND hwnd = nullptr;

};
