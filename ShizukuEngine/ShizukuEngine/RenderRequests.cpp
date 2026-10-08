#include "RenderRequests.h"

void RenderRequests::InitRender(ComPtr<ID3D12Device> device) {

	HRESULT hr;
	//ルートシグネチャを作成
	rootSignature = CreateRootSignature(device.Get(), hr);
	//PSOの作成
	pipelineState = CreatePipelineStateDesc(device.Get(), rootSignature, hr);

}

void RenderRequests::DrawRequestsSubmission(DrawRequest drawRequest) {
	mDrawRequests.push_back(drawRequest);
}


void RenderRequests::RenderAllRequests(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
	// 3Dリクエストと2Dリクエストを分離
	// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
	std::vector<DrawRequest> request3D;
	std::vector<DrawRequest> request2D;

	for (auto& req : mDrawRequests) {
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
		Render3DTarget(request3D, commandList);
	}
	//描画関数の呼び出し
	if (!request2D.empty()) {//から出ない場合は実行をする
		Render2DTarget(request2D, commandList);
	}
	//たまったリストをクリアする
	mDrawRequests.clear();
}



//----------------------------------------
//3Dオブジェクトを対象とした描画リクエスト送信関数
//----------------------------------------

void RenderRequests::Render3DTarget(
	const std::vector<DrawRequest>& requests3D,
	ComPtr<ID3D12GraphicsCommandList> commandList
) {

	//レンダーリクエストがない場合は下の処理をスキップ
	if (requests3D.empty())return;

	//インスタンス描画
	std::vector<DrawRequest> sortedRequests = requests3D;
	//インスタンス描画をする際に使うインデックス
	std::vector<int> instanceIndex(sortedRequests.size());

	std::sort(sortedRequests.begin(), sortedRequests.end(),
		[](const DrawRequest& a, const DrawRequest& b) {
			// renderOrdeを最優先で描画（小さいほうを優先して描画）
			if (a.renderOrder != b.renderOrder)return a.renderOrder < b.renderOrder;

			//バッチ化処理
			if (a.model != b.model)return a.model < b.model;
			if (a.textureIndex != b.textureIndex)return a.textureIndex < b.textureIndex;
			return false;
		}
	);

	//ルートシグネチャの設定
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	//PSOの設定
	commandList->SetPipelineState(pipelineState.Get());

	int start = 0;
	//ソートの範囲内で描画のリクエストを作成していく
	while (start < (int)sortedRequests.size())
	{
		//ソートの一番最初の値を入れるheadに入れる
		const DrawRequest& head = sortedRequests[start];

		int count = 1;
		while (start + count < (int)sortedRequests.size())
		{
			//ソートの最初の次の要素移行をnextに代入
			const DrawRequest& next = sortedRequests[start + count];
			//モデルが違う場合またテクスチャが違うオーダーが違うなどがあったときはwhile分を抜ける
			//同じモデルをまとめて処理するために行う
			if (next.model != head.model ||
				next.textureIndex != head.textureIndex ||
				next.renderOrder != head.renderOrder) {
				break;
			}
			count++;
		}

		//トポロジーの設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (head.model) {
			//スタートのインデックスにインスタンス「スタート」のインデックスを渡す
			UINT startIndex = (UINT)(instanceIndex[start]);
			commandList->SetGraphicsRoot32BitConstant(5, startIndex, 0);
			head.model->Draw(
				commandList.Get(),
				head.textureIndex,
				(UINT)(count),
				startIndex
			);
		}

		start += count;

	}


}

//----------------------------------------
//2Dオブジェクトを対象とした描画リクエスト送信関数
//----------------------------------------
void RenderRequests::Render2DTarget(
	const std::vector<DrawRequest>& requests2D,
	ComPtr<ID3D12GraphicsCommandList> commandlist
) {

}
