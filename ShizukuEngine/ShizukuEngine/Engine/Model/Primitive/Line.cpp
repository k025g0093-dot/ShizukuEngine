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
	mPWvpResource = CreateBufferResource();

}
