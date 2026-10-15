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

	//レンダーリクエストの初期化
	mRenderRequests.CreateOrthographicMatrix(width, Height);
	mRenderRequests.InitRender(mDX12Context.GetDevice(),&mTextureManager,&mLightManager);

	//ImGuiの初期化
	mImGuiManager.Initialize(
		mWinApp.GetHwnd(),
		mDX12Context.GetDevice(),
		mDX12Context.GetCommandQueue(),
		mDX12Context.GetDescriptorHeap(),
		TextureManager::kImGuiReserved,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_UNKNOWN
	);


#ifdef _DEBUG
	mDebugLayer.SetupInfoQueue(mDX12Context.GetDevice());
#endif
}


int ShizukuEngine::LoadTexture(const std::string& filePath) {
	//画像ファイルの読み込む
	return mTextureManager.LoadTexture(filePath);
}

int ShizukuEngine::LoadModel(const std::string& filePath) {
	//画像ファイルの読み込む
	return mModelManager.Load(filePath);
}

void ShizukuEngine::Update() {}

//開放処理
ShizukuEngine::~ShizukuEngine() {}


void ShizukuEngine::PreDraw() 
{
	mDX12Context.PreDraw();
	mImGuiManager.Begin();
}

void ShizukuEngine::PostDraw() 
{


	//ライトのGUI（ImGuiのフレーム中に呼ぶ）
	mLightWindow.update(&mLightManager);
	mCameraWindow.update(&mCamera);

	Matrix4x4 view = mCamera.GetViewMatrix();
	Matrix4x4 proj = mCamera.GetProjectionMatrix((float)mWidth, (float)mHeight);
	mViewProjectionMatrix = Multiply(view, proj);
	Vector3 cameraTranslate = mCamera.transform.translate;
	mRenderRequests.RenderAllRequests(
		mDX12Context.GetCommandList(),
		mViewProjectionMatrix,
		cameraTranslate
	);

	mDX12Context.SetImGuiRenderTarget();
	mImGuiManager.End(mDX12Context.GetCommandList());

	mDX12Context.PostDraw();

	mImGuiManager.RenderPlatformWindows();

}

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
