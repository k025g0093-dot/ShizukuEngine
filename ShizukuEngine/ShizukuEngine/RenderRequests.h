#pragma once
#include "Model.h"
#include "PSO.h"
#include "Camera.h"
#include <algorithm> 
//ドローリクエストの構造体
struct DrawRequest {
	Model* model = nullptr;
	std::vector<Vector3>vertices;
	Vector4 color = { 1,1,1,1 };
	Vector3 rot = { 0,0,0 };
	Vector3 scale = { 1,1,1 };
	Vector3 pos = { 0,0,0 };//３Dのオブジェクトなどで使用
	Vector2 posV2 = { 0,0 };//2Dのテクスチャなどで使用
	float width = 0.0f;
	float height = 0.0f;
	int textureIndex = 0;
	bool isMesh = false;//3Dオブジェクト化どうかの確認
	bool isSprit = false;//スプライトかそうでないかの確認
	bool is2D = false;
	int lightId = -1;//ライトを付与する際のID
	int renderOrder = 1;//レンダーのオーダー
};
class RenderRequests {
public:

	void initRender(ComPtr<ID3D12Device>* device);

	//描画レンダーリクエスト
	void RenderAllRequests(ComPtr<ID3D12GraphicsCommandList> commandList);

	void DrawRequestsSubmission(DrawRequest drawRequest);

private://ヘルパー関数など

	//----------------------------------------
	//3Dオブジェクトを対象とした描画リクエスト送信関数
	//----------------------------------------

	void Render3DTarget(const std::vector<DrawRequest>& requests3D, ComPtr<ID3D12GraphicsCommandList> commandList);

	//----------------------------------------
	//2Dオブジェクトを対象とした描画リクエスト送信関数
	//----------------------------------------
	void Render2DTarget(const std::vector<DrawRequest>& requests2D, ComPtr<ID3D12GraphicsCommandList> commandList);


private://メンバ変数

	int mMaxDrawCount = 128;
	ComPtr<ID3D12RootSignature>  rootSignature;//ルートシグネチャ
	ComPtr<ID3D12PipelineState>  pipelineState;//パイプラインステート
	std::vector<DrawRequest> mDrawRequests;//ドローリクエストの変数

	Matrix4x4 mViewProjectionMatrix;

};
