#include"ShizukuEngine/ShizukuEngine.h"
#include <Windows.h>

const int32_t kClineWidth = 1280;
const int32_t kClineHeight = 720;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	ShizukuEngine* engine = new ShizukuEngine(kClineHeight, kClineWidth, L"ShizukuEngine");

	Vector3 pos{ 0,0,0 };
	Vector3 rot{ 0,0,0 };

	MSG msg{};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else {

			engine->Update();

			rot.x+=0.01f;

			engine->PreDraw();

			for (int i = 0; i < 10; i++) {
				engine->DrawTriangle({ pos.x + 0.1f * i,pos.y + 0.1f * i,pos.z + 0.1f * i }, rot, { 1,1,1 }, { 1,1,1,1 }, 0);
			}
			engine->PostDraw();


		}
	}

	delete engine;

	return 0;

}