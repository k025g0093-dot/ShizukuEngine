#pragma once
#include "Model/Model.h"
#include "DirectX/PSO.h"
#include "Camera/Camera.h"
#include "InstanceData.h"
#include <algorithm> 
#include<array>
#include "ShadowPass.h"
#include "Model/SkinningConstants.h"

//ドローリクエストの構造体
struct DrawRequest {
	Model* model = nullptr;
	std::vector<Vector3>vertices;
	BlendMode blendMode = kBlendModeNormal;
	Vector4 color = { 1,1,1,1 };
	Vector3 rot = { 0,0,0 };
	Vector3 scale = { 1,1,1 };
	Vector3 pos = { 0,0,0 };
	int textureIndex = 0;
	bool isMesh = false;//3Dオブジェクト化どうかの確認
	bool isSprit = false;//スプライトかそうでないかの確認
	bool is2D = false;
	bool isParticle = false;//パーティクルかどうかの確認
	int lightId = -1;//ライトを付与する際のID
	int renderOrder = 1;//レンダーのオーダー
};
//前方宣言
class TextureManager;
class LightManager;
class DX12Context;

class RenderRequests {
public:

	void InitRender(
		ComPtr<ID3D12Device> device,
		TextureManager* textureManager,
		LightManager* lightManager,
		DX12Context* dX12Context
	);

	//描画レンダーリクエスト
	void RenderAllRequests(
		ComPtr<ID3D12GraphicsCommandList> commandList,
		Matrix4x4 viewProjectionMatrix,
		Vector3 cameraTranslate,
		Matrix4x4 billBoard
		);

	void DrawRequestsSubmission(DrawRequest drawRequest);

	//正射影行列作成
	void CreateOrthographicMatrix(int width, int height);



private://ヘルパー関数など

	//----------------------------------------
	//3Dオブジェクトを対象とした描画リクエスト送信関数
	//----------------------------------------

	void Render3DTarget(const std::vector<DrawRequest>& requests3D, ComPtr<ID3D12GraphicsCommandList> commandList);

	//----------------------------------------
	//2Dオブジェクトを対象とした描画リクエスト送信関数
	//----------------------------------------
	void Render2DTarget(const std::vector<DrawRequest>& requests2D, ComPtr<ID3D12GraphicsCommandList> commandList,int instanceOffset);

	//----------------------------------------
	//インスタンスバッファーを再作成する関数
	//----------------------------------------
	void CreateInstanceBuffer(int DrawCount);

	//バッチのテクスチャをparam 2にセットする（範囲外は0番）
	void BindTexture(ID3D12GraphicsCommandList* commandList, int textureIndex);

private://メンバ変数

	int mLastRequestCount = 0;   //前フレームの描画リクエスト数
	int mLastDrawCallCount = 0;  //前フレームのドローコール数

	int mMaxDrawCount = 128;
	ComPtr<ID3D12RootSignature>  rootSignature;//ルートシグネチャ
	//ブレンドモードごとのPSO（添字にBlendModeを使う）
	std::array<ComPtr<ID3D12PipelineState>, kCountOfBlendMode> pipelineState3D;//3D用
	std::array<ComPtr<ID3D12PipelineState>, kCountOfBlendMode> pipelineStateSkinned3D;//アニメーションモデル用
	std::array<ComPtr<ID3D12PipelineState>, kCountOfBlendMode> pipelineStateParticle3D;//パーティクル用
	std::array<ComPtr<ID3D12PipelineState>, kCountOfBlendMode> pipelineState2D;//2D用
	TextureManager* mTextureManager = nullptr;
	LightManager* mLightManager = nullptr;
	DX12Context* mDX12Context = nullptr;

	std::vector<DrawRequest> mDrawRequests;//ドローリクエストの変数


	ComPtr<ID3D12Resource> mInstanceBuffer;
	ComPtr<ID3D12Resource> mSkinnedInstanceBuffer;

	ID3D12Device* mDevice = nullptr;


	InstanceData* mInstanceData = nullptr;
	ShadowPass mShadowPass;

	Matrix4x4 mViewProjectionMatrix{};
	Matrix4x4 mBillBoard{};
	Matrix4x4 mOrtho{};
	Matrix4x4* mMappedBoneMatrices{};
	Vector3 mCameraTranslate{};


};
