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

// パーティクル1粒分の情報
struct Particle {
	Vector3 position;    // 位置
	Vector3 velocity;    // 速度(1秒あたり)
	float lifeTime;      // 寿命(秒)
	float currentTime;   // 生まれてからの経過時間(秒)
	int textureIndex;    // 使うテクスチャ
};

// パーティクルの発生源
struct Emitter {
	Vector3 position = { 0.0f, 0.0f, 0.0f }; // 発生位置
	int spawnCount = 5;                      // 1回の発生で出す数
	float frequency = 0.05f;                 // 何秒ごとに発生させるか
	float frequencyTime = 0.0f;              // 前回の発生からの経過時間
};

// パーティクルの動き方の設定（GUIから変更する）
struct ParticleSettings {
	bool isEmitting = true;   // 発生させるか
	float gravity = -9.8f;    // 重力
	float sideSpeed = 2.0f;   // 横方向の速さの最大
	float minUpSpeed = 5.0f;  // 上向きの速さ(最小)
	float maxUpSpeed = 9.0f;  // 上向きの速さ(最大)
	float minLife = 1.5f;     // 寿命(最小)
	float maxLife = 3.0f;     // 寿命(最大)
	float size = 0.2f;        // 生まれた瞬間の大きさ
	int maxParticles = 1000;  // 同時に存在できる最大数
	int textureMode = 2;      // 0:テクスチャ1 1:テクスチャ2 2:交互
};

// 新しいパーティクルを1つ作る（上向きにランダムな速度で打ち出す）
Particle MakeNewParticle(
	std::mt19937& randomEngine,
	const Emitter& emitter,
	const ParticleSettings& settings,
	int textureIndex
) {
	std::uniform_real_distribution<float> distSide(-settings.sideSpeed, settings.sideSpeed);
	std::uniform_real_distribution<float> distUp(settings.minUpSpeed, settings.maxUpSpeed);
	std::uniform_real_distribution<float> distLife(settings.minLife, settings.maxLife);

	Particle particle{};
	particle.position = emitter.position;
	particle.velocity = { distSide(randomEngine), distUp(randomEngine), distSide(randomEngine) };
	particle.lifeTime = distLife(randomEngine);
	particle.currentTime = 0.0f;
	particle.textureIndex = textureIndex;
	return particle;
}

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
	int model = engine->LoadModel("resources/suzanne.obj");//失敗したら-1（DrawModelは何もしない）

	// ---------- FPSの履歴（グラフ表示用） ----------
	std::array<float, 120> fpsHistory{};
	int fpsHistoryOffset = 0;

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

			modelRotateY += 0.01f;

			engine->PreDraw();

			//---------- モデル ----------
			engine->DrawModel(
				model,
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, modelRotateY, 0.0f },
				{ 2.0f, 2.0f, 2.0f },
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				tex
			);

			//---------- 今までの図形（モデルと重ならないよう横にずらす） ----------
			for (int i = 0; i < 10; i++) {
				engine->DrawTriangle({ pos.x - 6.0f + 0.1f * i, pos.y + 0.1f * i, pos.z - 1.5f * i }, rot, { 1,1,1 }, { 1,1,1,1 }, tex);
				engine->DrawSphere({ pos.x + 6.0f + 0.1f * i, pos.y + 0.1f * i, pos.z + 2.0f * i }, rot, { 1,1,1 }, { 1,1,1,1 }, tex);
			}

			engine->DrawSprite({ 100.0f, 100.0f }, 0.0f, 256.0f, 256.0f, { 1,1,1,1 }, tex);

			engine->PostDraw();
		}
	}

	delete engine;

	return 0;
}
