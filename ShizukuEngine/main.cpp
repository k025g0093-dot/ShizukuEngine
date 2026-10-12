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

	// ---------- テクスチャ（パーティクル用に2枚） ----------
	const int textures[2] = {
		engine->LoadTexture("resources/uvChecker.png"),
		engine->LoadTexture("resources/monsterBall.png"),
	};

	// ---------- パーティクル ----------
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());
	Emitter emitter{};
	ParticleSettings settings{};
	std::vector<Particle> particles;
	int spawnedTotal = 0; // 交互にテクスチャを切り替えるための通し番号

	// ---------- FPSの履歴（グラフ表示用） ----------
	std::array<float, 120> fpsHistory{};
	int fpsHistoryOffset = 0;

	MSG msg{};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else {

			engine->Update();

			// ImGuiのフレーム開始もPreDrawの中で行われる
			engine->PreDraw();

			const ImGuiIO& io = ImGui::GetIO();
			// ウィンドウをつかんで止まった後などに一気に進まないよう上限をつける
			const float deltaTime = (io.DeltaTime < 0.1f) ? io.DeltaTime : 0.1f;

			fpsHistory[fpsHistoryOffset] = io.Framerate;
			fpsHistoryOffset = (fpsHistoryOffset + 1) % static_cast<int>(fpsHistory.size());

			// ==================== デバッグ情報 ====================
			ImGui::Begin("デバッグ情報");
			ImGui::Text("FPS : %.1f (%.2f ms)", io.Framerate, 1000.0f / io.Framerate);
			ImGui::PlotLines("##fps", fpsHistory.data(), static_cast<int>(fpsHistory.size()),
				fpsHistoryOffset, nullptr, 0.0f, 120.0f, ImVec2(0.0f, 50.0f));
			ImGui::Separator();
			ImGui::Text("メモリ(プロセス) : %.1f MB", GetProcessMemoryMB());
			ImGui::Text("パーティクル数 : %d", static_cast<int>(particles.size()));
			ImGui::End();

			// ==================== パーティクルの設定 ====================
			ImGui::Begin("パーティクル");
			ImGui::Checkbox("発生させる", &settings.isEmitting);
			ImGui::DragFloat3("発生位置", &emitter.position.x, 0.1f);
			ImGui::SliderInt("1回の発生数", &emitter.spawnCount, 1, 50);
			ImGui::SliderFloat("発生間隔(秒)", &emitter.frequency, 0.01f, 1.0f);
			ImGui::SliderFloat("重力", &settings.gravity, -30.0f, 0.0f);
			ImGui::SliderFloat("横の速さ", &settings.sideSpeed, 0.0f, 10.0f);
			ImGui::DragFloatRange2("上向きの速さ", &settings.minUpSpeed, &settings.maxUpSpeed, 0.1f, 0.0f, 30.0f);
			ImGui::DragFloatRange2("寿命(秒)", &settings.minLife, &settings.maxLife, 0.05f, 0.1f, 10.0f);
			ImGui::SliderFloat("大きさ", &settings.size, 0.05f, 1.0f);
			ImGui::SliderInt("最大数", &settings.maxParticles, 10, 5000);
			ImGui::Combo("テクスチャ", &settings.textureMode, "1 : uvChecker\0" "2 : monsterBall\0" "交互\0");
			if (ImGui::Button("全部消す")) {
				particles.clear();
			}
			ImGui::End();

			// ==================== パーティクルの更新 ====================
			// ---------- 発生 ----------
			if (settings.isEmitting) {
				emitter.frequencyTime += deltaTime;
				while (emitter.frequencyTime >= emitter.frequency) {
					emitter.frequencyTime -= emitter.frequency;
					for (int i = 0; i < emitter.spawnCount; i++) {
						if (static_cast<int>(particles.size()) >= settings.maxParticles) { break; }

						int textureIndex = (settings.textureMode == 2)
							? textures[spawnedTotal % 2]
							: textures[settings.textureMode];
						particles.push_back(MakeNewParticle(randomEngine, emitter, settings, textureIndex));
						spawnedTotal++;
					}
				}
			}

			// ---------- 移動 ----------
			for (Particle& particle : particles) {
				particle.velocity.y += settings.gravity * deltaTime;
				particle.position += particle.velocity * deltaTime;
				particle.currentTime += deltaTime;
			}

			// 寿命が尽きたものを削除
			std::erase_if(particles, [](const Particle& particle) {
				return particle.currentTime >= particle.lifeTime;
			});

			// ==================== 描画 ====================
			for (const Particle& particle : particles) {
				// 残り寿命の割合(1→0)で小さくしていく
				float remain = 1.0f - (particle.currentTime / particle.lifeTime);
				float size = settings.size * remain;
				engine->DrawSphere(
					particle.position,
					{ 0.0f, 0.0f, 0.0f },
					{ size, size, size },
					{ 1.0f, 1.0f, 1.0f, remain },
					particle.textureIndex
				);
			}

			engine->PostDraw();
		}
	}

	delete engine;

	return 0;
}
