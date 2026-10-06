#include "TriangleModel.h"

//三角形描画に当たり必要な初期化
void TraiangleModel::Initialaize(ComPtr<ID3D12Device>* device)
{

	//一つの三角形を描画するので頂点数は3
	const UINT maxVertices = 3;
	const UINT bufferSize = sizeof(VertexData) * maxVertices;
	mVertexCount = maxVertices;

	mPVertexResource = CreateBufferResource(device->Get(), bufferSize);
	mPVertexResource->Map(0, nullptr, reinterpret_cast<void**>(&mPVertexDatta));

	mVertexBufferView.BufferLocation = mPVertexResource->GetGPUVirtualAddress();
	mVertexBufferView.StrideInBytes = sizeof(VertexData);
	mVertexBufferView.SizeInBytes = bufferSize;

	// マテリアルのセットアップ（Mapしたまま保持してDrawのたびに色を書き換えられるようにする）
	mPMaterialResource = CreateBufferResource(device->Get(), Align256(sizeof(Material)));
	mPMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = { 1.0f,1.0f,1.0f,1.0f };
	materialData->enableLighting = false;//ライティングを受けないように変更
	materialData->uvTransform = MakeIdentity4x4();

	// ライトのセットアップ
	mPLightResource = CreateBufferResource(device->Get(), Align256(sizeof(LightData)));
	LightData* lightData = nullptr;
	mPLightResource->Map(0, nullptr, reinterpret_cast<void**>(&lightData));
	lightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	lightData->dirOrPos = { 0.0f, -1.0f, 0.0f };
	lightData->intensity = 1.0f;
	mPLightResource->Unmap(0, nullptr);

}

void TraiangleModel::UpdateVertices
(
	const Vector3& positions,
	const Vector2& texcoord,
	const Vector3& normal,
	int index
){

	if (!mPVertexDatta || index < 0 || static_cast<uint32_t>(index) >= mVertexCount)return;

	mPVertexDatta[index].position = { positions.x,positions.y,positions.z,1.0f };
	mPVertexDatta[index].texcord = texcoord;
	mPVertexDatta[index].normal = { normal.x,normal.y,normal.x };


}

void TraiangleModel::Draw(int textureIndex,
	UINT instanceCount,
	UINT startInstanceLocation)
