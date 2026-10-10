#pragma once
#include "Model.h"
#include "PSO.h"
#include "Camera.h"
#include "InstanceData.h"
#include <algorithm> 
//ドローリクエストの構造体
struct DrawRequest {
	Model* model = nullptr;
	std::vector<Vector3>vertices;
	Vector4 color = { 1,1,1,1 };
	Vector3 rot = { 0,0,0 };
	Vector3 scale = { 1,1,1 };
	Vector3 pos = { 0,0,0 };
	int textureIndex = 0;
	bool isMesh = false;//3Dオブジェクト化どうかの確認
	bool isSprit = false;//スプライトかそうでないかの確認
	bool is2D = false;
	int lightId = -1;//ライトを付与する際のID
	int renderOrder = 1;//レンダーのオーダー
};
//前方宣言
class TextureManager;

class RenderRequests {
public:

	void InitRender(
		ComPtr<ID3D12Device> device,
		TextureManager* textureManager
	);

	//描画レンダーリクエスト
	void RenderAllRequests(ComPtr<ID3D12GraphicsCommandList> commandList,Matrix4x4 viewProjectionMatrix);

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

	int mMaxDrawCount = 128;
	ComPtr<ID3D12RootSignature>  rootSignature;//ルートシグネチャ
	ComPtr<ID3D12PipelineState>  pipelineState3D;//3D用パイプラインステート
	ComPtr<ID3D12PipelineState>  pipelineState2D;//2D用パイプラインステート
	TextureManager* mTextureManager = nullptr;
	std::vector<DrawRequest> mDrawRequests;//ドローリクエストの変数


	ComPtr<ID3D12Resource> mInstanceBuffer;

	ID3D12Device* mDevice = nullptr;

	InstanceData* mInstanceData = nullptr;
	InstanceData* mMappedInstanceBuffer = nullptr;

	Matrix4x4 mViewProjectionMatrix;
	Matrix4x4 mOrtho;
};
