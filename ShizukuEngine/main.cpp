#include"ShizukuEngine/ShizukuEngine.h"
#include <Windows.h>

const int32_t kClineWidth = 1280;
const int32_t kClineHeight = 720;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	ShizukuEngine* engine = new ShizukuEngine(kClineHeight, kClineWidth, L"ShizukuEngine");

	MSG msg{};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else {

			engine->Update();


			engine->PreDraw();

			engine->PostDraw();


		}
	}

	delete engine;

	return 0;

}