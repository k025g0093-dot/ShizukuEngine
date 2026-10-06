#pragma once
#include "Model.h"
#include "DX12Context.h"
#include "PSO.h"


//ドローリクエストの構造体
struct DrawRequest;

class RenderRequests {
public:

	void initRender(ComPtr<ID3D12Device>* device);

	//描画レンダーリクエスト
	void RenderAllRequests();

	//----------------------------------------
	//3Dオブジェクトを対象とした描画リクエスト送信関数
	//----------------------------------------

	void Render3DTarget(const std::vector<DrawRequest>& requests3D);

	//----------------------------------------
	//2Dオブジェクトを対象とした描画リクエスト送信関数
	//----------------------------------------
	void Render2DTarget(const std::vector<DrawRequest>& requests2D);

private:

	int mMaxDrawCount = 128;
	std::vector<DrawRequest> mDrawRequests;//レンダーリクエスト用の変数
	ComPtr<ID3D12RootSignature>  rootSignature;//ルートシグネチャ
	ComPtr<ID3D12PipelineState>  pipelineState;//パイプラインステート
	DX12Context dx12Context;

};
