#include "ShizukuEngine.h"

using namespace std;
//インスタンス初期化
ShizukuEngine* ShizukuEngine::mInstance = nullptr;

void ShizukuEngine::GetInstance(int Height, int width, wstring WinName) {
	if (mInstance == nullptr) {
		//インスタンスがない場合はnewをする

		mInstance = new ShizukuEngine(Height, width, WinName);
	}
}

ShizukuEngine::ShizukuEngine(int Height, int width, wstring WinName){
	mHeight = Height;
	mWidth = width;
	mWinApp.InitWindow(Height, width, WinName);
	Initialize(Height,width);

}

//初期化関数
void ShizukuEngine::Initialize(int Height, int width) {

	// COM は Windows 機能を使うために先に初期化しておく
	HRESULT hrCo = CoInitializeEx(0, COINIT_MULTITHREADED);

	// ログ用フォルダの作成とログ初期化
	std::filesystem::create_directory("logs");
	InitializeLog();
#ifdef _DEBUG
	mDebugLayer.EnableDebugLayer();
#endif

	mDX12Context.InitDXGIFactory(mWinApp.GetHwnd());
	mDX12Context.CreateCommandObjects(mWinApp.GetHwnd(), mHeight, mWidth);

	mRenderRequests.InitRender(mDX12Context.GetDevice());



#ifdef _DEBUG
	mDebugLayer.SetupInfoQueue(mDX12Context.GetDevice());
#endif


}


void ShizukuEngine::Update() {}

//開放処理
ShizukuEngine::~ShizukuEngine() {}


void ShizukuEngine::PreDraw() 
{
	mDX12Context.PreDraw();
}

void ShizukuEngine::PostDraw() 
{

	Matrix4x4 view = mCamera.GetViewMatrix();
	Matrix4x4 proj = mCamera.GetProjectionMatrix((float)mHeight, (float)mWidth);
	mViewProjectionMatrix = Multiply(view, proj);

	mRenderRequests.RenderAllRequests(mDX12Context.GetCommandList(), mViewProjectionMatrix);

	//ここで未来のレンダーリクエスト関数を使用
	//順番の前後には注意そこを間違えると描画されなくなる

	mDX12Context.PostDraw();
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

