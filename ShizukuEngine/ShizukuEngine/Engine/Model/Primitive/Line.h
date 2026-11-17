#pragma once
#include "Allinclude/AllVector.h"
#include <wrl.h>
#include <d3d12.h>
#include <vector>

using Microsoft::WRL::ComPtr;

class Line
{

public:
	void Initialize(ID3D12Device* device);

	void Clear();

	void Add(const Vector3& from, const Vector3& to, const Vector4& color);

	void Draw(ID3D12GraphicsCommandList* cmdList, const Matrix4x4& viewProj);

private:

	struct LineVertex {
		Vector4 position;
		Vector4 color;
	};

	ComPtr<ID3D12Resource> mPVertexResource;
	ComPtr<ID3D12Resource> mPWvpResource;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
	LineVertex* mPMappedVertexData = nullptr;
	Matrix4x4* mPWvpData = nullptr;
	UINT mMaxVertices = 0;
	std::vector<LineVertex> mVertices;

};

