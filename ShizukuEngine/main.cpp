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

struct Player {
	Vector3 pos;
	Vector3 rot;
	Vector3 scal;
};
struct Enemy {
	Vector3 pos;
	Vector3 rot;
	Vector3 scal;
};

void DebugPrintHit(bool isHit) {
	if (isHit) {
		OutputDebugStringA("[Collision] Hit!\n");
	}
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	ShizukuEngine* engine = new ShizukuEngine(kClineWidth, kClineHeight, L"LE2B_29_ヤマト_ユウヤ");

	//---------- 読み込み ----------
	int tex = engine->LoadTexture("resources/uvChecker.png");
	int model = engine->LoadModel("resources/fbx/Dragon 2.5_fbx.fbx");//失敗したら-1（DrawModelは何もしない）
	int skinnedModel = engine->LoadSkinnedModel("resources/Flair/Flair.dae");

	//1体目用のアニメを2つ読み込む（どちらも毎フレーム進む）
	int animeFlair = engine->LoadAnimation("resources/Flair/Flair.dae", skinnedModel);
	int animeBreak = engine->LoadAnimation("resources/new/Breakdance 1990.dae", skinnedModel);
	int currentAnime = animeFlair;//今描いているアニメ（切り替えフラグ）

	int skinnedModel2 = engine->LoadSkinnedModel("resources/new/Breakdance 1990.dae");
	int anime2 = engine->LoadAnimation("resources/new/Breakdance 1990.dae", skinnedModel2);

	assert(skinnedModel != -1 && "スキンモデルの読み込みに失敗");
	int terrain = engine->LoadModel("resources/terrain/terrain.obj");
	int particle = engine->LoadModel("resources/paticle/fence.obj");
	int skyDome = engine->LoadModel("resources/skyDome/skyDome.fbx");

	Emitter emitter{};
	emitter.position = { 0.0f, -2.0f, 0.0f };
	emitter.count = 1;
	emitter.velocityRange = { 0.5f, 2.0f, 0.5f };
	emitter.frequency = 0.2f;
	emitter.color = { 0.0f,0.50f,0.50f,1.0f };
	int file = engine->CreateEmitter(emitter, particle, emitter.color); // あとは毎フレーム自動で出て、動いて、描かれる

	//上向きの場（範囲の中だけ粒が上に流れる）
	AccelerationField field{};
	field.acceleration = { 0.0f, 9.0f, 0.0f };
	field.area.min = { -2.0f, -2.0f, -2.0f };
	field.area.max = { 2.0f, 2.0f,  2.0f };
	engine->CreateAccelerationField(field, file);

	std::vector<OBB>playerObb{};
	std::vector<OBB>enemyObb{};
	Player player;
	Enemy enemy;

	player.scal={ 1,1,1 };
	enemy.scal={ 1,1,1 };

	const float kMoveSpeed = 0.05f;

	Vector3 pos{ 0,0,0 };

	//モデルを回して立体感を確認する用
	float modelRotateY = 0.0f;

	SoundData soundData = engine
		->LoadSound("resources/fanfare.wav");


	MSG msg{};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else {

			engine->Update();

			//1キーと2キーで描くアニメを切り替える
			Input input = engine->GetInput();
			if (input.GetKeyDown('1')) { currentAnime = animeFlair; }
			if (input.GetKeyDown('2')) { currentAnime = animeBreak; }

			playerObb = engine->CreateAnimationModelOBBsCollision(
				skinnedModel, currentAnime,
				player.pos, player.rot, player.scal
			);
			enemyObb = engine->CreateAnimationModelOBBsCollision(
				skinnedModel2, anime2,
				enemy.pos, enemy.rot, enemy.scal
			);

			DebugPrintHit(engine->IsAnimationOBBsToAnimationOBBs(playerObb, enemyObb));


			if (input.GetKey('W')) { player.pos.z += kMoveSpeed; }
			if (input.GetKey('S')) { player.pos.z -= kMoveSpeed; }
			if (input.GetKey('A')) { player.pos.x -= kMoveSpeed; }
			if (input.GetKey('D')) { player.pos.x += kMoveSpeed; }

			modelRotateY += 0.0f;

			engine->PreDraw();

			//---------- モデル ----------
			engine->DrawModel(
				skyDome,
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, modelRotateY, 0.0f },
				{ 1.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f, 1.0f }
			);

			engine->DrawModel(
				terrain,
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, modelRotateY, 0.0f },
				{ 2.0f, 2.0f, 2.0f },
				{ 1.0f, 1.0f, 1.0f, 1.0f }
			);

			engine->DrawSkinnedModel(
				skinnedModel,
				currentAnime,//フラグで選んだアニメの姿勢で描く
				player.pos, player.rot, player.scal,
				{ 1.0f, 1.0f, 1.0f, 1.0f }
			);

			engine->DrawSkinnedModel(
				skinnedModel2,
				anime2,
				enemy.pos, enemy.rot, enemy.scal,
				{ 1.0f, 1.0f, 1.0f, 1.0f }

			);

			engine->PostDraw();

			if(input.GetKeyDown(VK_SPACE)){
				engine->SoundPlay(soundData);
			}

			if (input.GetKeyDown(VK_ESCAPE)) {
				break;
			}
		}
	}

	delete engine;

	return 0;
}