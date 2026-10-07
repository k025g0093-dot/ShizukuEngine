#include "RenderRequests.h"

void RenderRequests::initRender(ComPtr<ID3D12Device>* device) {

	HRESULT hr;
	pipelineState = CreatePipelineStateDesc(device->Get(), rootSignature, hr);

}


void RenderRequests::RenderAllRequests(DX12Context dx12Context)
{

	// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // 3Dリクエストと2Dリクエストを分離
	// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
	std::vector<DrawRequest> request3D;
	std::vector<DrawRequest> request2D;

	for (const auto& req : mDrawRequests) {
		if (req.isSprit) {
			//2Dへプッシュバック
			request2D.push_back(req);
		}
		else {
			//3Dへプッシュバック
			request3D.push_back(req);
		}
	}

	//描画関数の呼び出し
	if (!request3D.empty()) {//から出ない場合は実行をする
		Render3DTarget(request3D, dx12Context);
	}

	//描画関数の呼び出し
	if (!request2D.empty()) {//から出ない場合は実行をする
		Render2DTarget(request2D, dx12Context);
	}

	//たまったリストをクリアする
	mDrawRequests.clear();

}



//----------------------------------------
//3Dオブジェクトを対象とした描画リクエスト送信関数
//----------------------------------------

void RenderRequests::Render3DTarget(
	const std::vector<DrawRequest>& requests3D,
	DX12Context dx12Context
) {

	//レンダーリクエストがない場合は下の処理をスキップ
	if (requests3D.empty())return;

	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

	commandList = dx12Context.GetCommandList();

	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->SetPipelineState(pipelineState.Get());


}

//----------------------------------------
//2Dオブジェクトを対象とした描画リクエスト送信関数
//----------------------------------------
void RenderRequests::Render2DTarget(
	const std::vector<DrawRequest>& requests2D,
	DX12Context dx12Context
) {

}
