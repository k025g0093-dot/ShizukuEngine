#include "RenderRequests.h"
#include "TextureManager.h"
#include "LightManager.h"

void RenderRequests::InitRender(
	ComPtr<ID3D12Device> device,
	TextureManager* textureManager,
	LightManager* lightManager
) {

	mDevice = device.Get();
	mTextureManager = textureManager;
	mLightManager = lightManager;

	HRESULT hr;
	//ルートシグネチャを作成
	rootSignature = CreateRootSignature(mDevice, hr);
	//PSOの作成
	pipelineState3D = CreatePipelineStateDesc(mDevice, rootSignature, hr, PipelineType:: k3D);
	pipelineState2D = CreatePipelineStateDesc(mDevice, rootSignature, hr, PipelineType::k2D);

	CreateInstanceBuffer(mMaxDrawCount);
}

void RenderRequests::DrawRequestsSubmission(DrawRequest drawRequest) {
	mDrawRequests.push_back(drawRequest);
}


void RenderRequests::RenderAllRequests(
	ComPtr<ID3D12GraphicsCommandList> commandList,
	Matrix4x4 viewProjectionMatrix,
	Vector3 cameraTranslate
){
	// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
	// 3Dリクエストと2Dリクエストを分離
	// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
	std::vector<DrawRequest> request3D;
	std::vector<DrawRequest> request2D;

	mViewProjectionMatrix = viewProjectionMatrix;
	mCameraTranslate = cameraTranslate;

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

	int maxDrawCount = mMaxDrawCount;
	while (request3D.size() + request2D.size() > size_t(maxDrawCount)) {
		maxDrawCount = maxDrawCount * 2;
	}
	if (maxDrawCount > mMaxDrawCount) {
		CreateInstanceBuffer(maxDrawCount);
		mMaxDrawCount = maxDrawCount;
	}

	//描画関数の呼び出し
	if (!request3D.empty()) {//から出ない場合は実行をする
		Render3DTarget(request3D, commandList);
	}
	//描画関数の呼び出し
	if (!request2D.empty()) {//から出ない場合は実行をする
		Render2DTarget(request2D, commandList, (int)request3D.size());
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
	commandList->SetPipelineState(pipelineState3D.Get());

	mLightManager->Bind(commandList.Get());
	commandList->SetGraphicsRoot32BitConstants(6, 3, &mCameraTranslate, 0);

	for (int i = 0; i < (int)sortedRequests.size(); i++) {
		if (i >= mMaxDrawCount)break;

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
	}

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

		//インスタンスバッファーへのセット
		commandList->SetGraphicsRootShaderResourceView(
				1, mInstanceBuffer->GetGPUVirtualAddress());

		if (head.model) {
			//スタートのインデックスにインスタンス「スタート」のインデックスを渡す
			UINT startIndex = (UINT)(start);
			commandList->SetGraphicsRoot32BitConstant(5, startIndex, 0);
			BindTexture(commandList.Get(), head.textureIndex);
			head.model->Draw(
				commandList.Get(),
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

#pragma region 2Dのプライと描画リクエスト
void RenderRequests::Render2DTarget(
	const std::vector<DrawRequest>& requests2D,
	ComPtr<ID3D12GraphicsCommandList> commandList
	, int instanceOffset
) {

	//レンダーリクエストがない場合は下の処理をスキップ
	if (requests2D.empty())return;

	//インスタンス描画
	std::vector<DrawRequest> sortedRequests = requests2D;

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
	commandList->SetPipelineState(pipelineState2D.Get());


	for (int i = 0; i < (int)sortedRequests.size(); i++) {
		if (i >= mMaxDrawCount)break;

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

		
		mInstanceData[instanceOffset + i].World = MakeAffineMatrix(
			request.scale,
			request.rot,
			request.pos
		);

		mInstanceData[instanceOffset +i].WVP = Multiply(mInstanceData[instanceOffset +i].World, mOrtho);

	}

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
			UINT startIndex = (UINT)(instanceOffset +start);
			commandList->SetGraphicsRoot32BitConstant(5, startIndex, 0);
			BindTexture(commandList.Get(), head.textureIndex);
			head.model->Draw(
				commandList.Get(),
				(UINT)(count),
				startIndex
			);
		}

		start += count;

	}

}

#pragma endregion


void RenderRequests::CreateInstanceBuffer(int DrawCount) {
	mInstanceBuffer = CreateBufferResource(
		mDevice,
		sizeof(InstanceData) * DrawCount,
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_FLAG_NONE
	);
	//ちゃんと作られてるかの確認
	assert(mInstanceBuffer);

	mInstanceBuffer->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&mInstanceData)
	);
}

void RenderRequests::CreateOrthographicMatrix(int width,int height) {
	// 正射影行列を構築（画面座標系）
		mOrtho = MakeOrthographicMatrix(
		0.0f, 0.0f,
		static_cast<float>(width), static_cast<float>(height),
		0.1f, 100.0f
	);
}

void RenderRequests::BindTexture(ID3D12GraphicsCommandList* commandList, int textureIndex) {
	// 範囲外の番号は0番（白テクスチャ予定）に置き換える
	if (textureIndex < 0 || textureIndex >= mTextureManager->GetTextureCount()) {
		textureIndex = 0;
	}
	commandList->SetGraphicsRootDescriptorTable(2, mTextureManager->GetGPUHandle(textureIndex));
}
