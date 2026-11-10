#pragma once
#include<wrl.h>
#include <cassert>
#include <random>
#include<list>
#include "Allinclude/AllVector.h"
#include "Math/RandomEngine.h"
#include"DeltaTaime/Clock.h"
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
	float frequency=0.5f;
	float frequencyTime{};
};

struct AccelerationField {

	Vector3 acceleration;//加速度

	AABB area;//加速度がかかる範囲

};

class ParticleEmitter
{
public:

	ParticleEmitter();
	~ParticleEmitter();

	void CreateEmitter(const Emitter&emitter,Vector4 corol,bool isFixedColor=false );

	void SetAccelerationField(const AccelerationField&field);

	void Update(float timer);

	const std::list<Particle>& GetParticles() const { return particles; }
	
	const Emitter &GetEmitter() const { return mEmitter; }

	const AccelerationField& GetField()const { return mAccelerationField; }

	void SetEmitter(Emitter emitter) {
		mEmitter = emitter;
	}

	void SetField(AccelerationField field) {
		mAccelerationField = field;
	}

private://プライベート関数

	Particle MakeNewParticle(std::mt19937& randomEngine,const Emitter&emitter);

private://プライベート変数

	bool mIsFixedColor = false;

	std::list<Particle>particles;
	std::mt19937 mRandomEngine;
	Emitter mEmitter;
	AccelerationField mAccelerationField;
};

