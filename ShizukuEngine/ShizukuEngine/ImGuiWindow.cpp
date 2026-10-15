#include "ImGuiWindow.h"


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





