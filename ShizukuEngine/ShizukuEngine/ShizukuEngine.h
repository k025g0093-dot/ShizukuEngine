#pragma once
//ウィンドウアプリなどの作成ヘッダ
#include "WinApp.h"
#include "DXGI.h"
#include "DX12Context.h"
#include "RenderRequests.h"
//各種図形の関数が入ったヘッダ

#include"TriangleModel.h"

//ComPtr地獄だっきゃくのために必要
using Microsoft::WRL::ComPtr;
using namespace std;

struct VertexData;//ベクトルがたのデータが入ってる構造体
struct Material;//マテリアルの基本構造体

struct DrawRequest;

class ShizukuEngine
{
public:
	ShizukuEngine(int Height, int width, wstring WinName);//インスタンス関数
	~ShizukuEngine();//メモリ開放
	void GetInstance(int Height, int width, wstring WinName);//インスタンス取得関数


	//初期化関数
	void Initialize(int Height, int width);

	//更新処理
	void Update();

	//描画処理のpreとpost
	void PreDraw();
	void PostDraw();



	//--------------------------------------------------------------
	//三角形の描画関数（引数は、位置、回転、スケール、色、テクスチャです）
	//--------------------------------------------------------------
	void DrawTriangle(const Vector3& pos, const Vector3& rot, const Vector3& scale, const Vector4 color, int textureInd);


private://ヘルパー関数など内部関数がメイン

	//ヘルパー関数




private://各種変数などの初期化

	DX12Context mDX12Context{};
	WinApp mWinApp{};
	RenderRequests mRenderRequests{};

	ComPtr<ID3D12Device> mDevice;
	static ShizukuEngine* mInstance;

	std::unique_ptr<TriangleModel> mTriangleModel;
};

