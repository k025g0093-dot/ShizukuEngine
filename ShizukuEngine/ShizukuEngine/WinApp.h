#pragma once
#include <string>
#include "AllD3D12Include.h"



class WinApp
{

public:
	WinApp();
	~WinApp();
	void InitWindow(int Height, int width, std::wstring WinName);

	const HWND GetHwnd() const { return hwnd; }
	const HWND SetHwnd() {};

private:
	HWND hwnd = nullptr;

};
