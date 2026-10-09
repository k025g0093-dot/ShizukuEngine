#include "RenderRequests.h"

void RenderRequests::InitRender(ComPtr<ID3D12Device> device) {

	mDevice = device.Get();

	HRESULT hr;
	//ルートシグネチャを作成
	rootSignature = CreateRootSignature(mDevice, hr);
	//PSOの作成
	pipelineState = CreatePipelineStateDesc(mDevice, rootSignature, hr);

	CreateInstanceBuffer(mMaxDrawCount);
}

void RenderRequests::DrawRequestsSubmission(DrawRequest drawRequest) {
	mDrawRequests.push_back(drawRequest);
}


void RenderRequests::RenderAllRequests(ComPtr<ID3D12GraphicsCommandList> commandList,Matrix4x4 viewProjectionMatrix)
{
	// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
	// 3Dリクエストと2Dリクエストを分離
	// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
	std::vector<DrawRequest> request3D;
	std::vector<DrawRequest> request2D;

	mViewProjectionMatrix = viewProjectionMatrix;

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

#pragma region 3Dオブジェクトのリクエスト作成

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

	//描画上限を超えたリクエストを送ったときにドローカウントを上げてMapする

	int maxDrawCount = mMaxDrawCount;
	while (sortedRequests.size() > size_t( maxDrawCount)) {
		maxDrawCount = maxDrawCount * 2;
	}
	if (maxDrawCount > mMaxDrawCount) {
		CreateInstanceBuffer(maxDrawCount);
		mMaxDrawCount = maxDrawCount;
	}

	//ルートシグネチャの設定
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	//PSOの設定
	commandList->SetPipelineState(pipelineState.Get());

	int32_t currentInstanceCount = 0;
	std::vector<int>mInstanceIndex(sortedRequests.size());



	for (int i = 0; i < (int)sortedRequests.size(); i++) {
		if (currentInstanceCount >= mMaxDrawCount)break;

		mInstanceIndex[i] = currentInstanceCount;
		const DrawRequest& request = sortedRequests[i];

		// 安全ガード処理（既存コード）
		Vector3 safePos = request.pos;
		Vector3 safeRot = request.rot;
		Vector3 safeScale = request.scale;

		if (std::isnan(safePos.x) || std::isnan(safePos.y) || std::isnan(safePos.z)) {
			safePos = { 0.0f, 0.0f, 0.0f };
		}
		if (std::isnan(safeRot.x) || std::isnan(safeRot.y) || std::isnan(safeRot.z)) {
			safeRot = { 0.0f, 0.0f, 0.0f };
		}
		if (std::isnan(safeScale.x) || std::isnan(safeScale.y) || std::isnan(safeScale.z)) {
			safeScale = { 1.0f, 1.0f, 1.0f };
		}

		const float minScale = 0.0001f;
		if (std::abs(safeScale.x) < minScale) safeScale.x = (safeScale.x >= 0.0f) ? minScale : -minScale;
		if (std::abs(safeScale.y) < minScale) safeScale.y = (safeScale.y >= 0.0f) ? minScale : -minScale;
		if (std::abs(safeScale.z) < minScale) safeScale.z = (safeScale.z >= 0.0f) ? minScale : -minScale;

		mInstanceData[i].World= MakeAffineMatrix(safeScale,safeRot,safePos);
		mInstanceData[i].WVP= Multiply(mInstanceData[i].World, mViewProjectionMatrix);



		currentInstanceCount++;
	}
	if (currentInstanceCount == 0) return;


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

		commandList->SetGraphicsRootShaderResourceView(
				1, mInstanceBuffer->GetGPUVirtualAddress());

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

#pragma endregion


//----------------------------------------
//2Dオブジェクトを対象とした描画リクエスト送信関数
//----------------------------------------
void RenderRequests::Render2DTarget(
	const std::vector<DrawRequest>& requests2D,
	ComPtr<ID3D12GraphicsCommandList> commandlist
) {

}

void RenderRequests::CreateInstanceBuffer(int DrawCount) {
	mInstanceBuffer = CreateBufferResource(
		mDevice,
		sizeof(InstanceData) * DrawCount,
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_FLAG_NONE
	);
	mInstanceBuffer->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&mInstanceData)
	);

	//ちゃんと作られてるかの確認
	assert(mInstanceBuffer);

}
