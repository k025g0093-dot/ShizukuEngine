#include "Sphere.h"

constexpr float kPi = 3.14159265355f;

Sphere::~Sphere() = default;

void Sphere::Initialization(ComPtr<ID3D12Device> device) {

	const uint32_t kSubdivision = 16;
	mVertexCount = kSubdivision * kSubdivision * 4;
	mIndexCount = kSubdivision * kSubdivision * 6;

	mPVertexResource = CreateBufferResource(device.Get(), sizeof(VertexData) * mVertexCount);

	VertexData* vertexData = nullptr;
	mPVertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	const float kLonEvery = (kPi * 2.0f) / float(kSubdivision);
	const float kLatEvery = kPi / float(kSubdivision);

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		const float lat = -kPi / 2.0f + (kLatEvery * latIndex);
		const float nextLat = lat + kLatEvery;
		const float v = float(latIndex) / float(kSubdivision);
		const float nextV = float(latIndex + 1) / float(kSubdivision);

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			const uint32_t start = (latIndex * kSubdivision + lonIndex) * 4;
			const float lon = lonIndex * kLonEvery;
			const float nextLon = lon + kLonEvery;
			const float u = float(lonIndex) / float(kSubdivision);
			const float nextU = float(lonIndex + 1) / float(kSubdivision);

			vertexData[start + 0].position = { cosf(lat) * cosf(lon), sinf(lat), cosf(lat) * sinf(lon), 1.0f };
			vertexData[start + 0].texcord = { u, 1.0f - v };
			vertexData[start + 0].normal = { vertexData[start + 0].position.x, vertexData[start + 0].position.y, vertexData[start + 0].position.z };

			vertexData[start + 1].position = { cosf(nextLat) * cosf(lon), sinf(nextLat), cosf(nextLat) * sinf(lon), 1.0f };
			vertexData[start + 1].texcord = { u, 1.0f - nextV };
			vertexData[start + 1].normal = { vertexData[start + 1].position.x, vertexData[start + 1].position.y, vertexData[start + 1].position.z };

			vertexData[start + 2].position = { cosf(lat) * cosf(nextLon), sinf(lat), cosf(lat) * sinf(nextLon), 1.0f };
			vertexData[start + 2].texcord = { nextU, 1.0f - v };
			vertexData[start + 2].normal = { vertexData[start + 2].position.x, vertexData[start + 2].position.y, vertexData[start + 2].position.z };

			vertexData[start + 3].position = { cosf(nextLat) * cosf(nextLon), sinf(nextLat), cosf(nextLat) * sinf(nextLon), 1.0f };
			vertexData[start + 3].texcord = { nextU, 1.0f - nextV };
			vertexData[start + 3].normal = { vertexData[start + 3].position.x, vertexData[start + 3].position.y, vertexData[start + 3].position.z };
		}
	}
	mPVertexResource->Unmap(0, nullptr);

	mVertexBufferView.BufferLocation = mPVertexResource->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = sizeof(VertexData) * mVertexCount;
	mVertexBufferView.StrideInBytes = sizeof(VertexData);

	mPIndexResource = CreateBufferResource(device.Get(), sizeof(uint32_t) * mIndexCount);

	uint32_t* indexData = nullptr;
	mPIndexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			const uint32_t vertexStart = (latIndex * kSubdivision + lonIndex) * 4;
			const uint32_t indexStart = (latIndex * kSubdivision + lonIndex) * 6;

			indexData[indexStart + 0] = vertexStart + 0;
			indexData[indexStart + 1] = vertexStart + 1;
			indexData[indexStart + 2] = vertexStart + 2;
			indexData[indexStart + 3] = vertexStart + 1;
			indexData[indexStart + 4] = vertexStart + 3;
			indexData[indexStart + 5] = vertexStart + 2;
		}
	}
	mPIndexResource->Unmap(0, nullptr);

	mIndexBufferView.BufferLocation = mPIndexResource->GetGPUVirtualAddress();
	mIndexBufferView.SizeInBytes = sizeof(uint32_t) * mIndexCount;
	mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;

	mPMaterialResource = CreateBufferResource(device.Get(), Align256(sizeof(Material)));
	Material* materialData = nullptr;
	mPMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableNormalMap = 0;
	materialData->uvTransform = MakeIdentity4x4();
	mPMaterialResource->Unmap(0, nullptr);

}


void Sphere::Draw(ID3D12GraphicsCommandList* cmdList,UINT instanceCount, UINT startInstanceLocation) {
	cmdList->IASetVertexBuffers(0, 1, &mVertexBufferView);
	cmdList->IASetIndexBuffer(&mIndexBufferView);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	cmdList->SetGraphicsRootConstantBufferView(0, mPMaterialResource->GetGPUVirtualAddress());



	cmdList->DrawIndexedInstanced(mIndexCount, instanceCount, 0, 0, startInstanceLocation);
}
