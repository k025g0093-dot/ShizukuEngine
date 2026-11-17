#include "Line.h"
#include "DirectX/VertexResource.h"

void Line::Initialize(ID3D12Device* device) {

	mMaxVertices = 8192;
	const UINT bufferSize = sizeof(LineVertex) * mMaxVertices;

	mPVertexResource = CreateBufferResource(device, bufferSize);
	mPVertexResource->Map(0, nullptr, reinterpret_cast<void**>(&mPMappedVertexData));

	mVertexBufferView.BufferLocation = mPVertexResource->GetGPUVirtualAddress();
	mVertexBufferView.StrideInBytes = sizeof(LineVertex);
	mVertexBufferView.SizeInBytes = bufferSize;

	const UINT wvpSize = (sizeof(Matrix4x4) + 255) & ~255;
	mPWvpResource = CreateBufferResource(device,wvpSize);
	mPWvpResource->Map(0, nullptr, reinterpret_cast<void**>(&mPWvpData));

	mVertices.reserve(4096);

}

void Line::Clear() {
	mVertices.clear();
}

void Line::Add(
	const Vector3& from, 
	const Vector3& to,
	const Vector4& color
) {

	if (mVertices.size() + 2 > mMaxVertices)return;
	LineVertex v0, v1;
	v0.position = { from.x, from.y, from.z, 1.0f };
	v0.color = { color.x, color.y, color.z, color.w };
	v1.position = { to.x, to.y, to.z, 1.0f };
	v1.color = { color.x, color.y, color.z, color.w };

	mVertices.push_back(v0);
	mVertices.push_back(v1);

}

void Line::Draw(
	ID3D12GraphicsCommandList* cmdList,
	const Matrix4x4& viewProj
){
	if (mVertices.empty() || !mPMappedVertexData || !mPWvpData) return;

	memcpy(mPMappedVertexData, mVertices.data(), mVertices.size() * sizeof(LineVertex));
	*mPWvpData = viewProj;

	cmdList->SetGraphicsRootConstantBufferView(0, mPWvpResource->GetGPUVirtualAddress());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	cmdList->IASetVertexBuffers(0, 1, &mVertexBufferView);
	cmdList->DrawInstanced(static_cast<UINT>(mVertices.size()), 1, 0, 0);
}