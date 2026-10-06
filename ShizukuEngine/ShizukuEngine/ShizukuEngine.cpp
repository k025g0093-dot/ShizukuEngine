#include "ShizukuEngine.h"

using namespace std;
//インスタンス初期化
ShizukuEngine* ShizukuEngine::instance = nullptr;

void ShizukuEngine::GetInstance(int Height, int width, wstring WinName) {
	if (instance == nullptr) {
		//インスタンスがない場合はnewをする
		instance = new ShizukuEngine(Height, width, WinName);
	}
}

ShizukuEngine::ShizukuEngine(int Height, int width, wstring WinName){

	winApp.InitWindow(Height, width, WinName);
	Initialize(Height,width);
}


void ShizukuEngine::Initialize(int Height, int width) {

	dX12Context.InitDXGIFactory(winApp.GetHwnd(), &device);
	dX12Context.CreateCommandObjects(device, winApp.GetHwnd(), Height,width);
}

void ShizukuEngine::Run() {}

ShizukuEngine::~ShizukuEngine() {}

void ShizukuEngine::PreDraw() 
{
	dX12Context.PreDraw();
}

void ShizukuEngine::PostDraw() 
{
	dX12Context.PostDraw();
}