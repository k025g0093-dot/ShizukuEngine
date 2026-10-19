#include "ImGuiWindow.h"
#include "ModelManager.h"

#include <psapi.h>
#pragma comment(lib, "psapi.lib")

#include <cmath>
#include <cstring>


ImGuiUIWindow::ImGuiUIWindow() : show(true) {}
ImGuiUIWindow::~ImGuiUIWindow() {}
#ifdef USE_IMGUI
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
#endif // DEBUG


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
			if (ImGui::BeginMenu("ファイル")) {
				if (ImGui::MenuItem("保存")) {
					//ここでいろいろなライトの保存の実装だったりを行います
					//ModelManager::GetInstance()->SaveToFile();
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
			changed |= ImGui::DragFloat3("方向", &global.dirOrPos.x, 0.01f, -1.0f, 1.0f);
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
			std::string label = "ポイントライト " + std::to_string(i);

			if (ImGui::CollapsingHeader(label.c_str())) {
				LightData point = lm->GetLight(i);
				bool changed = false;
				changed |= ImGui::ColorEdit4("色", &point.color.x);
				changed |= ImGui::DragFloat3("位置", &point.dirOrPos.x, 0.1f);
				changed |= ImGui::DragFloat("強度", &point.intensity, 0.01f, 0.0f, 10.0f);

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
					point.type = 1;
					lm->SetLight(i, point);
				}
			}
			ImGui::PopID();
		}

		ImGui::Dummy(ImVec2(0.0f, 8.0f));

		// --- 追加ボタン ---
		if (ImGui::Button("+ ポイントライト追加")) {
			int newIndex = lm->AddLight();
			if (newIndex == -1) {
				ImGui::OpenPopup("LightFullPopup");
			}
		}
		if (ImGui::BeginPopup("LightFullPopup")) {
			ImGui::Text("これ以上ライトを追加できません（上限: %d）", lm->maxLight - 1);
			ImGui::EndPopup();
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
