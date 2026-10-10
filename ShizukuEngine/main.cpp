#include"ShizukuEngine/ShizukuEngine.h"
#include <Windows.h>

const int32_t kClineWidth = 1280;
const int32_t kClineHeight = 720;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	ShizukuEngine* engine = new ShizukuEngine(kClineWidth, kClineHeight,  L"ShizukuEngine");

	
	int tex= engine->LoadTexture("resources/uvChecker.png");


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


			engine->PreDraw();

			for (int i = 0; i < 10; i++) {
				engine->DrawTriangle({ pos.x + 0.1f * i,pos.y + 0.1f * i,pos.z - 1.5f * i }, rot, { 1,1,1 }, { 1,1,1,1 }, tex);
				engine->DrawSphere({ pos.x + 0.1f * i,pos.y + 0.1f * i,pos.z +2.0f * i }, rot, { 1,1,1 }, { 1,1,1,1 }, tex);
			}

			engine->DrawSprite({ 100.0f, 100.0f }, 0.0f, 256.0f, 256.0f, { 1,1,1,1 }, tex);

			engine->PostDraw();


		}
	}

	delete engine;

	return 0;

}