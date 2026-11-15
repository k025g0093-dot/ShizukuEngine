#pragma once
#include "Allinclude/AllVector.h"
#include "Math/AABB.h"

struct Particle
{
	Vector3 position;//位置
	Vector3 scale;//スケール
	Vector3 rotate;//回転
	Vector3 velocity;//速度
	Vector4 color;//色
	float lifeTime;//寿命
	float currentTime;//経過時間
};

struct Emitter
{
	Vector3 position{};//位置
	uint32_t count{};//一度に発生させる個数
	Vector3 direction{};//向き
	Vector3 velocityRange{};//広がる幅
	Vector4 color;//色範囲
	float frequency = 0.5f;
	float frequencyTime{};
};

struct AccelerationField {
	Vector3 acceleration;//加速度
	AABB area;//加速度がかかる範囲
};

struct PerView
{
	Matrix4x4 viewProjection;
	Matrix4x4 billboardMatrix;
};