#include "ShizukuEngine.h"

using namespace std;
//インスタンス初期化
ShizukuEngine* ShizukuEngine::mInstance = nullptr;

void ShizukuEngine::GetInstance(int width, int Height,wstring WinName) {
	if (mInstance == nullptr) {
		//インスタンスがない場合はnewをする
		mInstance = new ShizukuEngine(Height, width, WinName);
	}
}

ShizukuEngine::ShizukuEngine(int width, int Height,  wstring WinName){
	mHeight = Height;
	mWidth = width;
	mWinApp.InitWindow(width, Height, WinName);
	Initialize(width,Height);
}

//初期化関数
void ShizukuEngine::Initialize( int width,int Height) {

	// COM は Windows 機能を使うために先に初期化しておく
	HRESULT hrCo = CoInitializeEx(0, COINIT_MULTITHREADED);
	// ログ用フォルダの作成とログ初期化
	std::filesystem::create_directory("logs");
	InitializeLog();

	mClock.InitializeFixFPS();

#ifdef _DEBUG
	mDebugLayer.EnableDebugLayer();
#endif
	//ファクトリーとobjectの初期化
	mDX12Context.InitDXGIFactory(mWinApp.GetHwnd());
	mDX12Context.CreateCommandObjects(mWinApp.GetHwnd(), mHeight, mWidth);
	//テクスチャマネジャの初期化
	mTextureManager.Initialize(
		mDX12Context.GetDevice(),
		mDX12Context.GetDescriptorHeap(),
		mDX12Context.GetCommandList()
	);

	mTextureManager.LoadTexture("resources/white1x1.png");

	mLightManager.Initialize(mDX12Context.GetDevice());
	mModelManager.Initialize(mDX12Context.GetDevice(),&mTextureManager);

	mRenderTexture.CreateSceneRtv(
		mDX12Context.GetDevice(),
		&mTextureManager,
		width,
		Height
	);

	//レンダーリクエストの初期化
	mRenderRequests.CreateOrthographicMatrix(width, Height);
	mRenderRequests.InitRender(
		mDX12Context.GetDevice(),
		&mTextureManager,
		&mLightManager,
		&mDX12Context,
		&mRenderTexture);


	//ImGuiの初期化
#ifdef USE_IMGUI

	mImGuiManager.Initialize(
		mWinApp.GetHwnd(),
		mDX12Context.GetDevice(),
		mDX12Context.GetCommandQueue(),
		mDX12Context.GetDescriptorHeap(),
		TextureManager::kImGuiReserved,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_UNKNOWN
	);
#endif // USE_IMGUI


#ifdef _DEBUG
	mDebugLayer.SetupInfoQueue(mDX12Context.GetDevice());
#endif
}


int ShizukuEngine::LoadTexture(const std::string& filePath) {
	//画像ファイルの読み込む
	return mTextureManager.LoadTexture(filePath);
}

int ShizukuEngine::LoadModel(const std::string& filePath) {
	//モデルファイルの読み込む
	return mModelManager.Load(filePath);
}

int ShizukuEngine::LoadSkinnedModel(const std::string& filePath) {
	//スキン読み込み
	return mModelManager.LoadSkinnedModel(filePath);
}

void ShizukuEngine::OnFileDropped(const std::wstring& path) {

	std::filesystem::path filePath(path);
	std::string ext = filePath.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext == ".png" || ext == ".jpg") {
		std::string pathStr = ConvertString(path); // パスを文字列に変換する
		OutputDebugStringA(("Texture Loaded: " + pathStr + "\n").c_str());
	}
	else {

		std::string dir = filePath.parent_path().string();
		std::string filename = filePath.filename().string();
		std::string modelPath = "resources/" + dir + "/" + filename;

		int model = mModelManager.Load(modelPath);
		int skinnedModel = mModelManager.LoadSkinnedModel(modelPath);

		if (model !=-1|| skinnedModel!=-1) {

		}

	}
}

int ShizukuEngine::CreateEmitter(const Emitter& emitter, int modelIndex,Vector4 corol,bool isFixedColor) {

	auto particleEmitter = std::make_unique<ParticleEmitter>();
	particleEmitter->CreateEmitter(emitter, corol,isFixedColor);

	mEmitters.push_back({ std::move(particleEmitter),modelIndex });
	return static_cast<int>(mEmitters.size()) - 1;
}


int ShizukuEngine::CreateAccelerationField(const AccelerationField& field, int emitterIndex) {
	if (emitterIndex < 0 || emitterIndex >= mEmitters.size()) {
		return -1; // 無効なインデックスの場合はエラーコードを返す
	}
	mEmitters[emitterIndex].emitter->SetAccelerationField(field);
	return 0; // 成功
}

int ShizukuEngine::LoadAnimation(const std::string& animationPath, int skinnedModel)
{
	SkinnedModel* model = mModelManager.GetSkinned(skinnedModel);
	if (model == nullptr)return -1;

	auto animationModel = std::make_unique<Animation>();
	animationModel->Initialize(animationPath, model);
	mAnimations.push_back(std::move(animationModel));

	auto animator = std::make_unique<Animator>();
	animator->Initialize(mAnimations.back().get());
	mAnimator.push_back(std::move(animator));

	mRenderRequests.EnsureSkinnedCapacity(static_cast<int>(mAnimator.size()));

	return static_cast<int>(mAnimations.size())-1;
}


void ShizukuEngine::Update() {

	mClock.Update();
	const float deltaTime = mClock.GetDeltaTime();

	mInput.Update();

	for (EmitterSlot& slot : mEmitters) {
		slot.emitter->Update(deltaTime);
	}

	for (auto& animator : mAnimator){
		animator->UpdateAnimation(deltaTime);
	}


}

//開放処理
ShizukuEngine::~ShizukuEngine() {}


void ShizukuEngine::PreDraw() 
{
	mDX12Context.PreDraw();
#ifdef USE_IMGUI
	mImGuiManager.Begin();
#endif // USE_IMGUI

}

#pragma region postDraw

void ShizukuEngine::PostDraw() 
{

	DrawEmitter();
#ifdef USE_IMGUI

	//ライトのGUI（ImGuiのフレーム中に呼ぶ）
	mLightWindow.update(&mLightManager);
	mCameraWindow.update(&mCamera);
	mModelWindow.update(&mModelManager);
	mContentBrowser.update(&mTextureManager);

	//for (size_t i = 0; i < mEmitters.size();i++) {
	//	mImGuiParticleEmitter.Update(mEmitters[i].modelIndex,mEmitters[i].emitter.get());
	//}

	mViewportWindow.update(&mRenderTexture,&mEntityManager,&mContentBrowser);
	mPlayViewportWindow.update(&mRenderTexture);

	DebugStats stats{};
	//stats.drawRequestCount = mRenderRequests.GetLastRequestCount();
	//stats.drawCallCount = mRenderRequests.GetLastDrawCallCount();
	stats.textureCount = mTextureManager.GetTextureCount();
	stats.modelCount = mModelManager.GetModelCount();
	stats.lightCount = mLightManager.GetActiveLightCount();
	mDebugWindow.Update(stats);
#endif // USE_IMGUI

	Matrix4x4 view = mCamera.GetViewMatrix();
	Matrix4x4 proj = mCamera.GetProjectionMatrix((float)mWidth, (float)mHeight);
	mViewProjectionMatrix = Multiply(view, proj);
	Vector3 cameraTranslate = mCamera.transform.translate;
	Matrix4x4 billBoard = mCamera.GetBillboardMatrix();

	for (int it = 0; it < mAnimator.size(); it++) {
		mRenderRequests.SetBoneMatrices(mAnimator[it]->GetAnimationMatrix(), it);
	}

	mRenderRequests.RenderAllRequests(
		mDX12Context.GetCommandList(),
		mViewProjectionMatrix,
		cameraTranslate,
		billBoard
	);

	mDX12Context.SetImGuiRenderTarget();

#ifdef USE_IMGUI
	mImGuiManager.End(mDX12Context.GetCommandList());
#endif // USE_IMGUI

	mDX12Context.PostDraw();


#ifdef USE_IMGUI
	mImGuiManager.RenderPlatformWindows();
#endif // USE_IMGUI

	//mClock.UpdateFixFPS();

}

#pragma endregion

#pragma region 三角形描画関数

void ShizukuEngine::DrawTriangle
(
	const Vector3& pos,
	const Vector3& rot, 
	const Vector3& scale,
	const Vector4 color,
	int textureInd
) {
	if (!mTriangleModel) {
		mTriangleModel = std::make_unique<TriangleModel>();
		mTriangleModel->Initialization(mDX12Context.GetDevice());
	}
	DrawRequest req;
	req.model = mTriangleModel.get();
	req.pos = pos;//位置を渡す
	req.rot = rot;//回転度を渡す
	req.scale = scale;//スケールを渡す
	req.color = color;//色を渡す
	req.textureIndex = textureInd;//テクスチャのインデックスを渡す
	req.isMesh = false;//メッシュかの確認
	mRenderRequests.DrawRequestsSubmission(req);
}
#pragma endregion

#pragma region 球体描画関数

void ShizukuEngine::DrawSphere
(
	const Vector3& pos,
	const Vector3& rot, 
	const Vector3& scale,
	const Vector4 color,
	int textureInd
) {
	if (!mSphereModel) {
		mSphereModel = std::make_unique<Sphere>();
		mSphereModel->Initialization(mDX12Context.GetDevice());
	}
	DrawRequest req;
	req.model = mSphereModel.get();
	req.pos = pos;//位置を渡す
	req.rot = rot;//回転度を渡す
	req.scale = scale;//スケールを渡す
	req.color = color;//色を渡す
	req.textureIndex = textureInd;//テクスチャのインデックスを渡す
	req.lightId = 1;
	req.isMesh = false;//メッシュかの確認
	mRenderRequests.DrawRequestsSubmission(req);
}
#pragma endregion

#pragma region 画像描画関数

void ShizukuEngine::DrawSprite
(
	const Vector2& pos, 
	const float& rot,
	const float& width, 
	const float& height,
	const Vector4 color,
	int textureInd
) {
	if (!mSprite) {
		mSprite = std::make_unique<Sprite>();
		mSprite->Initialization(mDX12Context.GetDevice());
	}
	
	DrawRequest req;
	req.model = mSprite.get();
	req.pos = { pos.x,pos.y,0 };
	req.rot = { 0,0,rot };
	req.scale = { width,height,0 };
	req.color = color;
	req.textureIndex = textureInd;
	req.lightId = 1;
	req.isSprit = true;
	mRenderRequests.DrawRequestsSubmission(req);
}
#pragma endregion

#pragma region 3Dモデル描画関数

void ShizukuEngine::DrawModel
(
	int modelIndex,
	const Vector3& pos,
	const Vector3& rot,
	const Vector3& scale,
	const Vector4 color,
	int textureIndx
) {
	//番号からモデルを借りる（読み込み失敗の-1や範囲外ならnullptrが返る）
	MeshModel* model = mModelManager.Get(modelIndex);
	if (!model) { return; }

	DrawRequest req;
	req.model = model;
	req.pos = pos;//位置を渡す
	req.rot = rot;//回転度を渡す
	req.scale = scale;//スケールを渡す
	req.color = color;//色を渡す
	if(textureIndx==-1){
		req.textureIndex = model->GetTextureIndex();
	}
	else {
		req.textureIndex = textureIndx;//テクスチャのインデックスを渡す
	}
	mRenderRequests.DrawRequestsSubmission(req);
}
#pragma endregion

#pragma region パーティクル用モデル描画関数

void ShizukuEngine::DrawModelParticle
(
	int modelIndex,
	const Vector3& pos,
	const Vector3& rot,
	const Vector3& scale,
	const Vector4 color,
	int textureIndx
) {
	//番号からモデルを借りる（読み込み失敗の-1や範囲外ならnullptrが返る）
	MeshModel* model = mModelManager.Get(modelIndex);
	if (!model) { return; }

	DrawRequest req;
	req.model = model;
	req.pos = pos;//位置を渡す
	req.rot = rot;//回転度を渡す
	req.scale = scale;//スケールを渡す
	req.color = color;//色を渡す
	req.isParticle = true;
	if(textureIndx==-1){
		req.textureIndex = model->GetTextureIndex();
	}
	else {
		req.textureIndex = textureIndx;//テクスチャのインデックスを渡す
	}
	mRenderRequests.DrawRequestsSubmission(req);
}
#pragma endregion

#pragma region スキンモデル用描画関数

void ShizukuEngine::DrawSkinnedModel
(
	int modelIndex,
	int boneSlot,
	const Vector3& pos,
	const Vector3& rot,
	const Vector3& scale,
	const Vector4 color,
	int textureIndx
) {
	//番号からモデルを借りる（読み込み失敗の-1や範囲外ならnullptrが返る）
	SkinnedModel* model = mModelManager.GetSkinned(modelIndex);

	if (!model) { return; }

	DrawRequest req;
	req.model = model;
	req.boneSlot = boneSlot;
	req.pos = pos;//位置を渡す
	req.rot = rot;//回転度を渡す
	req.scale = scale;//スケールを渡す
	req.color = color;//色を渡す
	req.isSkinnedModel = true;
	if(textureIndx==-1){
		req.textureIndex = model->GetTextureIndex();
	}
	else {
		req.textureIndex = textureIndx;//テクスチャのインデックスを渡す
	}
	mRenderRequests.DrawRequestsSubmission(req);
}
#pragma endregion

#pragma region パーティクル用モデル描画関数

void ShizukuEngine::DrawEmitter() 
{

	for (const EmitterSlot& slot : mEmitters) {
		for (const Particle& particle : slot.emitter->GetParticles()) {

			DrawModelParticle(

				slot.modelIndex,
				particle.position,
				particle.rotate,
				particle.scale,
				particle.color

			);
		}
	}
}
#pragma endregion
