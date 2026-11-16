#include "ImGuiWindow.h"
#include "Model/ModelManager.h"
#include "ImGuiContentBrowser.h"
#include "Particle/ParticleEmitter.h"

#include <psapi.h>
#pragma comment(lib, "psapi.lib")

#include <cmath>
#include <cstring>

#ifdef USE_IMGUI

ImGuiUIWindow::ImGuiUIWindow() : show(true) {}
ImGuiUIWindow::~ImGuiUIWindow() {}
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);


void ImGuiUIWindow::update() {
	if (begin("Default Window")) {
	}
	end();

}

void ImGuiUIWindow::Show() {
	show = true;
}

// ImGuiWindow.cpp
bool ImGuiUIWindow::begin(std::string name, ImGuiWindowFlags flags)
{
#ifdef USE_IMGUI
	if (!show) return false;

	// 前フレームで制限範囲外に出ていたら位置を矯正
	if (m_needsClamp) {
		ImGui::SetNextWindowPos(m_nextPos, ImGuiCond_Always);
		m_needsClamp = false;
	}

	return ImGui::Begin(name.c_str(), &show, flags);
#else
	return false;
#endif
}



void ImGuiUIWindow::end()
{
#ifdef USE_IMGUI
	if (m_enableClamp) {
		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 size = ImGui::GetWindowSize();
		ImVec2 clamped;
		clamped.x = (std::max)(m_clampMin.x, (std::min)(pos.x, m_clampMax.x - size.x));
		clamped.y = (std::max)(m_clampMin.y, (std::min)(pos.y, m_clampMax.y - size.y));
		if (clamped.x != pos.x || clamped.y != pos.y) {
			m_nextPos = clamped;
			m_needsClamp = true;
		}
	}

	ImGui::End();
#endif
}

//=============================
//IGStartupWindowの実装
//=============================


void ImGuiDebugWindow::Update(const DebugStats& stats) {
#ifdef USE_IMGUI
	const ImGuiIO& io = ImGui::GetIO();

	//FPSの履歴を更新（リングバッファ）
	mFpsHistory[mHistoryOffset] = io.Framerate;
	mHistoryOffset = (mHistoryOffset + 1) % kHistoryCount;

	if (begin("デバッグ")) {


		//---------- パフォーマンス ----------
		if (ImGui::CollapsingHeader("パフォーマンス", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("FPS : %.1f (%.2f ms)", io.Framerate, 1000.0f / io.Framerate);
			ImGui::PlotLines("##fps", mFpsHistory, kHistoryCount, mHistoryOffset,
				nullptr, 0.0f, 144.0f, ImVec2(0.0f, 50.0f));

			//このプロセスが使っているメモリ
			PROCESS_MEMORY_COUNTERS_EX counters{};
			if (GetProcessMemoryInfo(GetCurrentProcess(),
				reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters), sizeof(counters))) {
				ImGui::Text("メモリ : %.1f MB", counters.PrivateUsage / (1024.0f * 1024.0f));
			}
		}

		//---------- 描画 ----------
		if (ImGui::CollapsingHeader("描画", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("描画リクエスト : %d", stats.drawRequestCount);
			ImGui::Text("ドローコール   : %d", stats.drawCallCount);
		}

		//---------- リソース ----------
		if (ImGui::CollapsingHeader("リソース", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("テクスチャ : %d", stats.textureCount);
			ImGui::Text("モデル     : %d", stats.modelCount);
			ImGui::Text("ライト     : %d", stats.lightCount);
		}
	}
	end();
#endif // USE_IMGUI
}




void ImGuiLightManagerWindow::update(LightManager* lm) {
#ifdef USE_IMGUI

	if (begin("ライティングのシーン管理", ImGuiWindowFlags_MenuBar))
	{

		//メニューバー
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("ライトファイル")) {
				if (ImGui::MenuItem("現在のライトの値を保存")) {
					lm->SaveLightsToFile();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}


		// --- グローバルライト（常に1個だけ、固定） ---
		if (ImGui::CollapsingHeader("グローバルライト")) {
			LightData global = lm->GetLight(0);
			bool changed = false;
			changed |= ImGui::ColorEdit4("色", &global.color.x);
			changed |= ImGui::DragFloat3("方向", &global.direction.x, 0.01f, -1.0f, 1.0f);
			changed |= ImGui::DragFloat("強度", &global.intensity, 0.01f, 0.0f, 10.0f);
			if (changed) lm->SetLight(0, global);
		}

		ImGui::Dummy(ImVec2(0.0f, 6.0f));
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0.0f, 6.0f));

		// --- ポイントライト一覧（使われているものだけ表示） ---
		for (int i = 1; i < lm->maxLight; i++) {
			if (!lm->IsLightActive(i)) continue;

			ImGui::PushID(i);
			std::string label{};
			if (lm->GetLight(i).type == 1)
			{
				label = "ポイントライト " + std::to_string(i);
			}
			else {
				label = "スポットライト " + std::to_string(i);
			}

			if (ImGui::CollapsingHeader(label.c_str())) {
				LightData light = lm->GetLight(i);
				bool changed = false;
				changed |= ImGui::ColorEdit4("色", &light.color.x);
				changed |= ImGui::DragFloat3("位置", &light.position.x, 0.1f);
				changed |= ImGui::DragFloat("強度", &light.intensity, 0.01f, 0.0f, 10.0f);


				if (lm->GetLight(i).type == 2) {
					changed |= ImGui::DragFloat3("向き", &light.direction.x, 0.01f, -1.0f, 2.0f);
					changed |= ImGui::DragFloat("範囲", &light.distance, 0.1f, 0.1f, 100.0f);
					changed |= ImGui::DragFloat("減衰率", &light.decay, 0.01f, 0.0f, 10.0f);

					changed |= ImGui::SliderAngle("スポットライト角度", &light.cosAngle, 1.0f, 89.0f);
				}

				if (ImGui::Button("ギズモで選択")) {
					lm->SetSelectedLight(i);
				}
				ImGui::SameLine();
				if (ImGui::Button("削除")) {
					lm->RemoveLight(i);
					ImGui::PopID();
					continue; // 削除直後にPopIDしたので、このまま次のループへ
				}

				if (changed) {
					lm->SetLight(i, light);
				}

			}
			ImGui::PopID();
		}

		ImGui::Dummy(ImVec2(0.0f, 8.0f));

		ImGui::Combo("ライトの追加", &mNewLightType, "ポイントライト\0スポットライト\0"); {

			// --- 追加ボタン ---
			if (ImGui::Button("+ ライト追加")) {

				if (mNewLightType == 0) {
					int newIndex = lm->AddPointLight();
					if (newIndex == -1) {
						ImGui::OpenPopup("LightFullPopup");
					}
				}
				else {
					int newIndex = lm->AddSpotLight();
					if (newIndex == -1) {
						ImGui::OpenPopup("LightFullPopup");
					}
				}
			}
			if (ImGui::BeginPopup("LightFullPopup")) {
				ImGui::Text("これ以上ライトを追加できません（上限: %d）", lm->maxLight - 1);
				ImGui::EndPopup();
			}
		}

	}
	end();

#endif // _DEBUG
}



//=============================
//カメラ操作ウィンドウ
//=============================
void ImGuiCameraWindow::update(Camera* camera) {
#ifdef USE_IMGUI
	if (!camera) { return; }

	if (begin("カメラ")) {
		ImGui::DragFloat3("位置", &camera->transform.translate.x, 0.1f);

		//回転は中身がラジアンなので、表示と入力だけ度に変換する
		const float kRadToDeg = 180.0f / 3.14159265f;
		float rotateDeg[3] = {
			camera->transform.rotate.x * kRadToDeg,
			camera->transform.rotate.y * kRadToDeg,
			camera->transform.rotate.z * kRadToDeg,
		};
		if (ImGui::DragFloat3("回転(度)", rotateDeg, 0.5f)) {
			camera->transform.rotate.x = rotateDeg[0] / kRadToDeg;
			camera->transform.rotate.y = rotateDeg[1] / kRadToDeg;
			camera->transform.rotate.z = rotateDeg[2] / kRadToDeg;
		}

		//near < far を保つように範囲を制限する
		float nearPlane = camera->GetNearPlane();
		float farPlane = camera->GetFarPlane();
		if (ImGui::DragFloat("Near", &nearPlane, 0.01f, 0.01f, farPlane - 0.01f)) {
			camera->SetNearPlane(nearPlane);
		}
		if (ImGui::DragFloat("Far", &farPlane, 10.0f, nearPlane + 0.01f, 100000.0f)) {
			camera->SetFarPlane(farPlane);
		}

		if (ImGui::Button("初期位置に戻す")) {
			*camera = Camera{};
		}
	}
	end();
#endif // USE_IMGUI
}



//=============================
//モデル操作ウィンドウ
//=============================
void ImGuiModelWindow::update(ModelManager* modelManager) {
#ifdef USE_IMGUI
	if (!modelManager) { return; }

	if (begin("モデル")) {
		const int modelCount = modelManager->GetModelCount();
		if (modelCount == 0) {
			ImGui::TextDisabled("読み込まれたモデルはありません");
		}

		for (int i = 0; i < modelCount; i++) {
			MeshModel* model = modelManager->Get(i);
			if (!model) { continue; }

			ImGui::PushID(i);
			const std::string label = "モデル " + std::to_string(i);
			if (ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
				const std::string texturePath = model->GetTexturePath();
				ImGui::Text("テクスチャ : %s", texturePath.empty() ? "(なし)" : texturePath.c_str());

				Vector4 color = model->GetColor();
				if (ImGui::ColorEdit4("色", &color.x)) {
					model->SetColor(color);
				}

				bool enableLighting = model->GetEnableLighting();
				if (ImGui::Checkbox("ライティング", &enableLighting)) {
					model->SetEnableLighting(enableLighting);
				}

				if (ImGui::Button("色を戻す")) {
					model->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
				}
			}
			ImGui::PopID();
		}
	}
	end();
#endif // USE_IMGUI
}

void ImGuiViewportWindow::update(
	RenderTexture* RenderTexture,
	EntityManager* entityManager,
	ImGuiContentBrowser* imGuiContentBrowser
) {

#ifdef USE_IMGUI

	mEntityManager = entityManager;
	mRenderTexture = RenderTexture;
	mImGuiContentBrowser = imGuiContentBrowser;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });

	if (ImGui::Begin("デバックビューポート", nullptr,
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoBringToFrontOnFocus)) {

		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		ImVec2 imageScreenPos = ImGui::GetCursorScreenPos();

		ImGui::Image(
			(ImTextureID)mRenderTexture->GetSceneSrvGpuHandle().ptr,
			viewportSize,
			ImVec2(0, 0),
			ImVec2(1, 1),
			ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
			ImVec4(0.0f, 0.0f, 0.0f, 0.0f)
		);

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
				const wchar_t* itemPath = (const wchar_t*)payload->Data;
				auto contentBrowser = mImGuiContentBrowser;
				//if(contentBrowser&&contentBrowser->onFileDorop)
			}
		}

		const auto playState = mEntityManager->GetPlayState();
		const char* stateLabel = playState == EntityManager::PlayState::editing ? "EDIT"
			: playState == EntityManager::PlayState::playing ? "PLAY"
			: "PAUSE";
		ImGui::SetCursorScreenPos(ImVec2(imageScreenPos.x + 10.0f, imageScreenPos.y + 10.0f));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.07f, 0.88f));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 6.0f));
		ImGui::BeginChild("##ViewportPlayControls", ImVec2(245.0f, 32.0f), ImGuiChildFlags_Borders);
		ImGui::TextColored(
			playState == EntityManager::PlayState::editing ? ImVec4(0.70f, 0.70f, 0.70f, 1.0f)
			: ImVec4(0.35f, 0.85f, 0.45f, 1.0f),
			"%s", stateLabel);
		ImGui::SameLine();
		if (ImGui::Button("Play##Viewport", ImVec2(50.0f, 0.0f))
			&& playState == EntityManager::PlayState::editing) {
		}
		ImGui::SameLine();
		if (ImGui::Button(playState == EntityManager::PlayState::paused
			? "Resume##Viewport" : "Pause##Viewport", ImVec2(58.0f, 0.0f))
			&& playState != EntityManager::PlayState::editing) {
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop##Viewport", ImVec2(48.0f, 0.0f))
			&& playState != EntityManager::PlayState::editing) {
		}
		ImGui::EndChild();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();

	}



	ImGui::End();
	ImGui::PopStyleVar(2);
#endif
}


void ImGuiPlayViewportWindow::update(RenderTexture* RenderTexture) {

#ifdef USE_IMGUI

	mRenderTexture = RenderTexture;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });

	if (ImGui::Begin("プレイビューポート", nullptr,
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoBringToFrontOnFocus)) {

		ImVec2 viewportSize = ImGui::GetContentRegionAvail();

		ImGui::Image(
			(ImTextureID)mRenderTexture->GetSceneSrvGpuHandle().ptr,
			viewportSize,
			ImVec2(0, 0),
			ImVec2(1, 1),
			ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
			ImVec4(0.0f, 0.0f, 0.0f, 0.0f)
		);

	}
	ImGui::End();
	ImGui::PopStyleVar(2);
#endif
}


void ImGuiParticleEmitter::Update(
	int model,
	ParticleEmitter* particleEmitter
) {

	if (begin("エミッター")) {

		if (ImGui::CollapsingHeader("エミッター")) {

			Emitter emitter = particleEmitter->GetEmitter();
			bool changed = false;
			changed |= ImGui::DragFloat3("位置", &emitter.position.x, 0.1f);
			changed |= ImGui::DragScalar("発生数", ImGuiDataType_U32, &emitter.count, 1.0f);
			changed |= ImGui::DragFloat3("広がる幅", &emitter.velocityRange.x, 0.01f, 0.0f, 10.0f);
			changed |= ImGui::DragFloat("発生区間", &emitter.frequency, 0.01f, 0.0f, 10.0f);
			changed |= ImGui::ColorEdit4("色", &emitter.color.x);

			ImGui::Dummy(ImVec2(0.0f, 6.0f));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0.0f, 6.0f));

			AccelerationField accelField = particleEmitter->GetField();
			changed |= ImGui::DragFloat3("加速幅", &accelField.acceleration.x, 0.01f, 0.0f, 10.0f);
			changed |= ImGui::DragFloat3("AABBのMIN", &accelField.area.min.x, 0.01f, -10.0f, 10.0f);
			changed |= ImGui::DragFloat3("AABBのMAX", &accelField.area.max.x, 0.01f, -10.0f, 10.0f);

			if (changed) {
				particleEmitter->SetEmitter(emitter);
				particleEmitter->SetField(accelField);
			}

		}
	}
	end();
}

#endif // DEBUG
