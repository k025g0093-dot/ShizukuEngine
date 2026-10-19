#pragma once
#include "ConvertString.h"
#include "LightManager.h"
#include "Camera.h"

class ModelManager;
#ifdef USE_IMGUI
#endif // USE_IMGUI

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "imguizmo.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


class ImGuiUIWindow
{
public:
    ImGuiUIWindow();
    ~ImGuiUIWindow();

    virtual void update();
    void Show();

protected:


    virtual bool begin(std::string name, ImGuiWindowFlags flags = 0);
    void end();
    bool show = true; 

    // ウィンドウ移動範囲の制限
    bool m_enableClamp = false;          // true で移動制限を有効化
    ImVec2 m_clampMin = { 0,0 };         // 制限範囲 (左上)
    ImVec2 m_clampMax = { 0,0 };         // 制限範囲 (右下)
    ImVec2 m_nextPos = { 0,0 };          // 次フレームで適用する位置
    bool m_needsClamp = false;
};


//デバッグウィンドウに表示する情報
struct DebugStats {
    int drawRequestCount = 0;  //積まれた描画リクエストの数
    int drawCallCount = 0;     //実際のドローコールの数（まとめた後）
    int textureCount = 0;      //読み込み済みテクスチャ数
    int modelCount = 0;        //読み込み済みモデル数
    int lightCount = 0;        //有効なライト数
};


class ImGuiDebugWindow: public ImGuiUIWindow
{
public:
    void Update(const DebugStats& stats);

private:
    static constexpr int kHistoryCount = 240;
    float mFpsHistory[kHistoryCount] = {};
    int mHistoryOffset = 0;
};

class ImGuiLightManagerWindow : public ImGuiUIWindow
{
public:
    void update(LightManager* lm);
};

//カメラの位置・回転などをGUIで操作する
class ImGuiCameraWindow : public ImGuiUIWindow
{
public:
    void update(Camera* camera);
};

//読み込んだモデルの色などをGUIで操作する
class ImGuiModelWindow : public ImGuiUIWindow
{
public:
    void update(ModelManager* modelManager);
};

class ImGuiViewportWindow : public ImGuiUIWindow
{
public:
    void update() override;
};

class ImGuiPlayViewportWindow : public ImGuiUIWindow
{
public:
    void update() override;
};

class ImGuiZmoWindow : public ImGuiUIWindow
{
public:

    int selectedIndex = -1;
	void update() override;
	void SetObjectsIndex(int index) { selectedIndex = index; }
};


