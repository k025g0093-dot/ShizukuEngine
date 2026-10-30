#pragma once
#include<wrl.h>
#include <cassert>
#include <random>
#include<list>
#include "Allinclude/AllVector.h"
#include "../Math/RandomEngine.h"
#include"../DeltaTaime/Clock.h"

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
	float frequency=0.5f;
	float frequencyTime{};
};



class ParticleEmitter
{
public:

	ParticleEmitter();
	~ParticleEmitter();

	void CreateEmitter(const Emitter&emitter);

	void Update(float timer);

	const std::list<Particle>& GetParticles() const { return particles; }

private://プライベート関数

	Particle MakeNewParticle(std::mt19937& randomEngine,const Emitter&emitter);

private://プライベート変数

	std::list<Particle>particles;
	std::mt19937 mRandomEngine;
	Emitter mEmitter;

};

