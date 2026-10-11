#pragma once
#include "AllVector.h"
#include <d3d12.h>
#include <vector>
#include "VertexResource.h"
#include "LightManager.h"


struct VertexData {
	Vector4 position;
	Vector2 texcord;
	Vector3 normal;
	Vector3 tangent;
};

struct Material {
    Vector4 color;
    int32_t enableLighting;
    int32_t enableNormalMap;
    Vector2 padding;
    Matrix4x4 uvTransform;
};



struct LightData;

class Model {

protected:
	Vector3 mPosition = {0.0f,0.0f,0.0f};
	Vector3 mRotation = { 0.0f,0.0f,0.0f };
	Vector3 mScale = { 0.0f,0.0f,0.0f };

    VertexData* mPVertexData = nullptr;
	UINT mVertexCount = 0;
	UINT mIndexCount = 0;

public:

    // Model.h の public: セクション内に追加
    const VertexData* GetVertexData() const { return mPVertexData; }
    UINT GetVertexCount() const { return mVertexCount; }

    virtual ~Model() {}

    UINT GetIndexCount() const { return mIndexCount; }

    void SetPosition(const Vector3& p) { mPosition = p; }
    void SetRotation(const Vector3& r) { mRotation = r; }
    void SetScale(const Vector3& s) { mScale = s; }

    Matrix4x4 GetWorldMatrix() const;

    // オプション：各モデルが独自の transform を設定する場合
    virtual void SetUVTransform(const Matrix4x4&) {}



    virtual void Draw(
        ID3D12GraphicsCommandList* cmdList,
        UINT instanceCount,
        UINT startInstanceLocation) = 0;

};
