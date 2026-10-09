#include "TriangleModel.h"

//三角形描画に当たり必要な初期化
void TriangleModel::Initialization(ComPtr<ID3D12Device> device)
{

	//一つの三角形を描画するので頂点数は3
	const UINT maxVertices = 3;
	const UINT bufferSize = sizeof(VertexData) * maxVertices;
	mVertexCount = maxVertices;

	mPVertexResource = CreateBufferResource(device.Get(), bufferSize);
	mPVertexResource->Map(0, nullptr, reinterpret_cast<void**>(&mPVertexData));

	mVertexBufferView.BufferLocation = mPVertexResource->GetGPUVirtualAddress();
	mVertexBufferView.StrideInBytes = sizeof(VertexData);
	mVertexBufferView.SizeInBytes = bufferSize;

	mPVertexData[0] = { { -0.5f, -0.5f, 0.0f,1 },{0.0f,1.0f}, { 0.0f, 0.0f, -1.0f } };
	mPVertexData[1] = { { 0.0f,  0.5f, 0.0f,1 }, { 0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
	mPVertexData[2] = { { 0.5f, -0.5f, 0.0f ,1}, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } };

	// マテリアルのセットアップ（Mapしたまま保持してDrawのたびに色を書き換えられるようにする）
	mPMaterialResource = CreateBufferResource(device.Get(), Align256(sizeof(Material)));
	mPMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = { 1.0f,1.0f,1.0f,1.0f };
	materialData->enableLighting = false;//ライティングを受けないように設定
	materialData->uvTransform = MakeIdentity4x4();

	// ライトのセットアップ
	mPLightResource = CreateBufferResource(device.Get(), Align256(sizeof(LightData)));
	LightData* lightData = nullptr;
	mPLightResource->Map(0, nullptr, reinterpret_cast<void**>(&lightData));
	lightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	lightData->dirOrPos = { 0.0f, -1.0f, 0.0f };
	lightData->intensity = 1.0f;
	mPLightResource->Unmap(0, nullptr);

}



void TriangleModel::Draw(
	ID3D12GraphicsCommandList* cmdList,
	UINT instanceCount,
	UINT startInstanceLocation)
{

	if (mVertexCount == 0 || !mPVertexResource)return;

	//トポロジーと頂点バッファのセット
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->IASetVertexBuffers(0, 1, &mVertexBufferView);

	//マテリアル
	cmdList->SetGraphicsRootConstantBufferView(0, mPMaterialResource->GetGPUVirtualAddress());

	// テクスチャセット
	//if (textureIndex >= 0) {
	//	cmdList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetGPUHandle(textureIndex));
	//}
	cmdList->DrawInstanced(3, instanceCount, 0, startInstanceLocation);
}
