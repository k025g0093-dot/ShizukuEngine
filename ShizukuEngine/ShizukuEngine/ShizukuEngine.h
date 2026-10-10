#pragma once
//ウィンドウアプリなどの作成ヘッダ
#include "WinApp.h"
#include "DXGI.h"
#include "DX12Context.h"
#include "RenderRequests.h"

#include "LogSistem.h"
#include "DebugLayer.h"
#include "Camera.h"

#include "TextureManager.h"

//各種図形の関数が入ったヘッダ
#include"TriangleModel.h"
#include "Sprite.h"
#include "Sphere.h"

//ComPtr地獄だっきゃくのために必要
using Microsoft::WRL::ComPtr;
using namespace std;

struct VertexData;//ベクトルがたのデータが入ってる構造体
struct Material;//マテリアルの基本構造体

struct DrawRequest;

class ShizukuEngine
{
public:

	//幅、高さの順で入れる
	ShizukuEngine(int width, int Height, wstring WinName);//インスタンス関数
	~ShizukuEngine();//メモリ開放

	void GetInstance(int width, int Height,  wstring WinName);//インスタンス取得関数


	//初期化関数
	void Initialize(int width,int Height);

	//更新処理
	void Update();

	//描画処理のpreとpost
	void PreDraw();
	void PostDraw();

	//--------------------------------------------------------------
	//画像ファイル読み込み関数
	//--------------------------------------------------------------
	int LoadTexture(const std::string& filePath);


	//--------------------------------------------------------------
	//三角形の描画関数（引数は、位置、回転、スケール、色、テクスチャです）
	//--------------------------------------------------------------
	void DrawTriangle(const Vector3& pos, const Vector3& rot, const Vector3& scale, const Vector4 color, int textureInd);
	
	//--------------------------------------------------------------
	//三角形の描画関数（引数は、位置、回転、スケール、色、テクスチャです）
	//--------------------------------------------------------------
	void DrawSphere(const Vector3& pos, const Vector3& rot, const Vector3& scale, const Vector4 color, int textureInd);
	
	//--------------------------------------------------------------
	//スプライトの描画関数（引数は、左上を起点に下位置、回転、スケール、色、テクスチャです）
	//--------------------------------------------------------------
	void DrawSprite(const Vector2& pos, const float& rot, const float & width,const float &height, const Vector4 color, int textureInd);


private://ヘルパー関数など内部関数がメイン

	//ヘルパー関数
	const Matrix4x4& GetViewProjectionMatrix() const { return mViewProjectionMatrix; }



private://各種変数などの初期化

	DX12Context mDX12Context{};
	WinApp mWinApp{};
	RenderRequests mRenderRequests{};
	DebugLayer mDebugLayer{};
	Camera mCamera{};
	TextureManager mTextureManager{};

	static ShizukuEngine* mInstance;


	std::unique_ptr<TriangleModel> mTriangleModel;
	std::unique_ptr<Sphere> mSphereModel;
	std::unique_ptr<Sprite> mSprite;

	Matrix4x4 mViewProjectionMatrix;
	int mHeight;
	int mWidth;
};

