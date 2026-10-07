#pragma once
#include "Model.h"
#include "DX12Context.h"
#include "PSO.h"
#include "Camera.h"

//ドローリクエストの構造体
struct DrawRequest;

class RenderRequests {
public:

	void initRender(ComPtr<ID3D12Device>* device);

	//描画レンダーリクエスト
	void RenderAllRequests(DX12Context dx12Context);

	//----------------------------------------
	//3Dオブジェクトを対象とした描画リクエスト送信関数
	//----------------------------------------

	void Render3DTarget(const std::vector<DrawRequest>& requests3D, DX12Context dx12Context);

	//----------------------------------------
	//2Dオブジェクトを対象とした描画リクエスト送信関数
	//----------------------------------------
	void Render2DTarget(const std::vector<DrawRequest>& requests2D, DX12Context dx12Context);

private:

	int mMaxDrawCount = 128;
	std::vector<DrawRequest> mDrawRequests;//レンダーリクエスト用の変数
	ComPtr<ID3D12RootSignature>  rootSignature;//ルートシグネチャ
	ComPtr<ID3D12PipelineState>  pipelineState;//パイプラインステート

	Matrix4x4 mViewProjectionMatrix;

};
