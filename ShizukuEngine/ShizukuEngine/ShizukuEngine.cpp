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

	mWinApp.InitWindow(Height, width, WinName);
	Initialize(Height,width);
}

//初期化関数
void ShizukuEngine::Initialize(int Height, int width) {

	mDX12Context.InitDXGIFactory(mWinApp.GetHwnd(), &mDevice);
	mDX12Context.CreateCommandObjects(mDevice, mWinApp.GetHwnd(), Height,width);
	mRenderRequests.initRender(&mDevice);
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

	mRenderRequests.RenderAllRequests(mDX12Context.GetCommandList());

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
		mTriangleModel->Initialization(&mDevice);
	}
	DrawRequest req;
	req.pos = pos;//位置を渡す
	req.rot = rot;//回転度を渡す
	req.scale = scale;//スケールを渡す
	req.color = color;//色を渡す
	req.textureIndex = textureInd;//テクスチャのインデックスを渡す
	req.isMesh = false;//メッシュかの確認
	mRenderRequests.DrawRequestsSubmission(req);
}

