#include "ShizukuEngine/ShizukuEngine.h"
#include <Windows.h>
#include <psapi.h>
#include <array>
#include <random>
#include <vector>
#include <imgui.h>

#pragma comment(lib, "psapi.lib")

const int32_t kClineWidth = 1280;
const int32_t kClineHeight = 720;

// このプロセスが使っているメモリ量(MB)を取得する
float GetProcessMemoryMB() {
	PROCESS_MEMORY_COUNTERS_EX counters{};
	BOOL isSucceeded = GetProcessMemoryInfo(
		GetCurrentProcess(),
		reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters),
		sizeof(counters)
	);
	if (!isSucceeded) { return 0.0f; }
	return static_cast<float>(counters.PrivateUsage) / (1024.0f * 1024.0f);
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	ShizukuEngine* engine = new ShizukuEngine(kClineWidth, kClineHeight, L"ShizukuEngine");

	//---------- 読み込み ----------
	int tex = engine->LoadTexture("resources/uvChecker.png");
	int model = engine->LoadModel("resources/bunny.obj");//失敗したら-1（DrawModelは何もしない）
	int terrain = engine->LoadModel("resources/terrain/terrain.obj");

	Vector3 pos{ 0,0,0 };

	//モデルを回して立体感を確認する用
	float modelRotateY = 0.0f;

	MSG msg{};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else {

			engine->Update();

			modelRotateY = 0.0f;

			engine->PreDraw();

			//---------- モデル ----------
			engine->DrawModel(
				model,
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, modelRotateY, 0.0f },
				{ 2.0f, 2.0f, 2.0f },
				{ 1.0f, 1.0f, 1.0f, 1.0f }
			);

			engine->DrawModel(
				terrain,
				{ 0.0f, -2.0f, 0.0f },
				{ 0.0f, modelRotateY, 0.0f },
				{ 2.0f, 2.0f, 2.0f },
				{ 1.0f, 1.0f, 1.0f, 1.0f }
			);

			//---------- 今までの図形（モデルと重ならないよう横にずらす） ----------
			for (int i = 0; i < 10; i++) {
				engine->DrawTriangle({ pos.x - 3.0f + 0.1f * i, pos.y + 0.1f * i, pos.z - 1.5f * i }, { 0,0,0 }, { 1,1,1 }, { 1,1,1,1 }, tex);
				engine->DrawSphere({ pos.x + 6.0f + 0.1f * i, pos.y + 0.1f * i, pos.z + 2.0f * i }, { 1,1,1 }, { 1,1,1 }, { 1,1,1,1 }, tex);
			}

			engine->DrawSprite({ 100.0f, 100.0f }, 0.0f, 256.0f, 256.0f, { 1,1,1,1 }, tex);

			engine->PostDraw();
		}
	}

	delete engine;

	return 0;
}
