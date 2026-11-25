#pragma once
//ウィンドウアプリなどの作成ヘッダ
#include "Core/WinApp.h"
#include "DirectX/DX12Context.h"
#include "Render/RenderRequests.h"
#include "Render/RenderTexture.h"

#include "Core/LogSistem.h"
#include "Core/Input.h"
#include "DirectX/DebugLayer.h"
#include "DirectX/ComputePipeline.h"
#include "Camera/Camera.h"

//マネジャクラス
#include "TextureManager/TextureManager.h"
#include "Light/LightManager.h"
#include "Model/ModelManager.h"

#include "GUI/ImGuiManager.h"
#include "GUI/ImGuiWindow.h"
#include "GUI/ImGuiContentBrowser.h"

//各種図形の関数が入ったヘッダ
#include"Model/Primitive/TriangleModel.h"
#include "Model/Sprite/Sprite.h"
#include "Model/Primitive/Sphere.h"
#include "Model/Primitive/Line.h"


#include "Particle/ParticleEmitter.h"//通常のパーティクル
#include "Particle/GPUParticleEmitter.h"//GPU駆動式パーティクル
#include "DeltaTaime/Clock.h"
#include "Entity/EntityManager.h"

//アニメーションクラス関係
#include "Model/3DAnimation/Animation.h"
#include "Model/3DAnimation/Animator.h"

#include "Sound/Sound.h"


using Microsoft::WRL::ComPtr;
using namespace std;

struct VertexData;//ベクトルがたのデータが入ってる構造体
struct Material;//マテリアルの基本構造体
struct SoundData;

struct DrawRequest;

class ShizukuEngine
{
public:

	//幅、高さの順で入れる
	ShizukuEngine(int width, int Height, wstring WinName);//インスタンス関数
	~ShizukuEngine();//メモリ開放

	void GetInstance(int width, int Height, wstring WinName);//インスタンス取得関数


	//初期化関数
	void Initialize(int width, int Height);

	//更新処理
	void Update();

	//描画処理のpreとpost
	void PreDraw();
	void PostDraw();

	//入力取得関数
	Input GetInput()const { return mInput; }

	//--------------------------------------------------------------
	//画像ファイル読み込み関数
	//--------------------------------------------------------------
	int LoadTexture(const std::string& filePath);

	//--------------------------------------------------------------
	//モデルファイル読み込み関数
	//--------------------------------------------------------------
	int LoadModel(const std::string& filePath);

	//--------------------------------------------------------------
	//音楽ファイル読み込み関数（対応してるのはWAV、MP3）
	//--------------------------------------------------------------
	SoundData LoadSound(const char *filePath);

	//--------------------------------------------------------------
	//音楽再生関数（対応してるのはWAV、MP3）
	//--------------------------------------------------------------
	void SoundPlay(SoundData soundData);

	//--------------------------------------------------------------
	//アニメーションスキン付きmodel読み込み関数
	//--------------------------------------------------------------
	int LoadSkinnedModel(const std::string& filePath);

	//--------------------------------------------------------------
	//アニメーションスキン付きmodel読み込み関数
	//--------------------------------------------------------------
	int LoadAnimation(const std::string& animationPath, int skinnedModel);
	
	//--------------------------------------------------------------
	//アニメーションの作成関数
	//--------------------------------------------------------------
	int CreateAnimator(int animation);

	//--------------------------------------------------------------
	//アニメーション変更作成関数
	//--------------------------------------------------------------
	void ChangeAnimation(int animators, int animation, bool isRooped = true);

	//--------------------------------------------------------------
	//パーティクルのフィールド作成関数
	//--------------------------------------------------------------
	int CreateAccelerationField(const AccelerationField& field, int emitterIndex);

	//--------------------------------------------------------------
	//エミッターの作成関数（引数はエミッターの情報とモデルの番号）
	// 色の固定ができますデフォルトはランダムが指定されています
	//--------------------------------------------------------------
	int CreateEmitter(const Emitter& emitter, int modelIndex, Vector4 color = { 1.0f,1.0f,1.0f,1.0f }, bool isFixedColor = false);

	//--------------------------------------------------------------
	//エミッターの作成関数（引数はエミッターの情報とモデルの番号）
	// 色の固定ができますデフォルトはランダムが指定されています
	//--------------------------------------------------------------
	int CreateGPUEmitter(const Emitter& emitter, int modelIndex, Vector4 color = { 1.0f,1.0f,1.0f,1.0f }, bool isFixedColor = false);

	
	

	Camera& GetCamera() { return mCamera; }

	//-----------------------------------描画関数-----------------------------------//


	//--------------------------------------------------------------
	//三角形の描画関数（引数は、位置、回転、スケール、色、テクスチャです）
	//--------------------------------------------------------------
	void DrawTriangle(const Vector3& pos, const Vector3& rot, const Vector3& scale, const Vector4 color = { 1.0f,1.0f,1.0f,1.0f }, int textureIndx = -1);

	//--------------------------------------------------------------
	//球体の描画関数（引数は、位置、回転、スケール、色、テクスチャです）
	//--------------------------------------------------------------
	void DrawSphere(const Vector3& pos, const Vector3& rot, const Vector3& scale, const Vector4 color = { 1.0f,1.0f,1.0f,1.0f }, int textureIndx = -1);

	//--------------------------------------------------------------
	//線分の描画関数（引数は、開始位置、最終地点　色）
	//--------------------------------------------------------------
	void DrawLine(const Vector3& start, const Vector3 end, const Vector4 color);

	//--------------------------------------------------------------
	//スプライトの描画関数（引数は、左上を起点に下位置、回転、スケール、色、テクスチャです）
	//--------------------------------------------------------------
	void DrawSprite(const Vector2& pos, const float& rot, const float& width, const float& height, const Vector4 color = { 1.0f,1.0f,1.0f,1.0f }, int textureIndx = -1);

	//--------------------------------------------------------------
	//モデルの描画関数（引数は、モデル番号、位置、回転、スケール、色、テクスチャです）テクスチャは宣言をしなくても平気です
	//--------------------------------------------------------------
	void DrawModel(int modelIndex, const Vector3& pos, const Vector3& rot, const Vector3& scale, const Vector4 color = { 1.0f,1.0f,1.0f,1.0f }, int textureIndx = -1);

	//--------------------------------------------------------------
	//モデルの描画関数（引数は、モデル番号、アニメーション値、位置、回転、スケール、色、テクスチャです）テクスチャは宣言をしなくても平気です
	//--------------------------------------------------------------
	void DrawSkinnedModel(
		int modelIndex, int boneSlot,
		const Vector3& pos, const Vector3& rot, const Vector3& scale,
		const Vector4 color = { 1.0f,1.0f,1.0f,1.0f },
		int textureIndx = -1);

	//--------------------------------------------------------------
	//モデルパーティクルの描画関数（引数は、モデル番号、位置、回転、スケール、色、テクスチャです）テクスチャは宣言をしなくても平気です
	//--------------------------------------------------------------
	void DrawModelParticle(int modelIndex, const Vector3& pos, const Vector3& rot, const Vector3& scale, const Vector4 color = { 1.0f,1.0f,1.0f,1.0f }, int textureIndx = -1);




	//-----------------------------------当たり判定関数-----------------------------------//

	//--------------------------------------------------------------
	//アニメーションモデルのコリジョンの作成関数
	//位置・回転・スケールは DrawSkinnedModel と同じ値を渡す
	//--------------------------------------------------------------
	std::vector<OBB> CreateAnimationModelOBBsCollision(int modelIndex, int boneSlot, const Vector3& pos, const Vector3& rot, const Vector3& scale);

	//--------------------------------------------------------------
	//アニメーションモデルどうしのOBB判定関数
	//--------------------------------------------------------------
	bool IsAnimationOBBsToAnimationOBBs(const std::vector<OBB>& obb1, const std::vector<OBB>& obb2);

	//--------------------------------------------------------------
	//アニメーションモデルOBBと通常のOBB判定関数
	//--------------------------------------------------------------
	bool IsAnimationOBBToOBB(const std::vector<OBB>& obb1, const OBB& obb2);

	//--------------------------------------------------------------
	//OBBとOBBの当たり判定関数
	//--------------------------------------------------------------
	bool IsOBBToOBB(const OBB& obb1, const OBB& obb2);

	//-----------------------------------getter、セッター関数-----------------------------------//




private://ヘルパー関数など内部関数がメイン

	//ヘルパー関数
	const Matrix4x4& GetViewProjectionMatrix() const { return mViewProjectionMatrix; }

	void DrawEmitter();

	void OnFileDropped(const std::wstring& path);

private://各種変数などの初期化

	DX12Context mDX12Context{};
	WinApp mWinApp{};
	RenderRequests mRenderRequests{};
	RenderTexture mRenderTexture{};
	EntityManager mEntityManager{};

	DebugLayer mDebugLayer{};
	Camera mCamera{};
	TextureManager mTextureManager{};
	LightManager mLightManager;
	Input mInput{};
	Sound* mSound{};

	std::vector<std::unique_ptr<Animation>> mAnimations;
	Animation mAnimation{};
	std::vector < std::unique_ptr< Animator>> mAnimator{};

	ModelManager mModelManager{};

#ifdef USE_IMGUI
	ImGuiManager mImGuiManager{};

	ImGuiLightManagerWindow mLightWindow{};//ライト操作用のGUI（確認用）
	ImGuiCameraWindow mCameraWindow{};//カメラ操作用のGUI（確認用）
	ImGuiModelWindow mModelWindow{};//モデルの色を操作するGUI（確認用）
	ImGuiViewportWindow mViewportWindow{};//レンダリング結果を表示するGUI（確認用デバック確認）
	ImGuiPlayViewportWindow mPlayViewportWindow{};//プレイビューポート用のGUI（ゲーム実行確認）
	ImGuiContentBrowser mContentBrowser;
	ImGuiDebugWindow mDebugWindow{};//デバック情報を載せたGUI
	ImGuiParticleEmitter mImGuiParticleEmitter{};

#endif // USE_IMGUI



	static ShizukuEngine* mInstance;


	std::unique_ptr<TriangleModel> mTriangleModel;
	std::unique_ptr<Sphere> mSphereModel;
	std::unique_ptr<Sprite> mSprite;
	std::unique_ptr<Line> mLine;


	struct EmitterSlot {
		std::unique_ptr<ParticleEmitter> emitter;
		int modelIndex = -1;//初期値として何もない場合は出さない
	};

	std::vector<EmitterSlot>mEmitters;

	std::vector<std::unique_ptr<GPUParticleEmitter>> mGPUEmitters;

	//デルタタイマーを取得するため
	Clock mClock;

	Matrix4x4 mViewProjectionMatrix;
	Vector3 mShadowCenter{};
	int mHeight;
	int mWidth;
};

