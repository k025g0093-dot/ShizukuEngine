#pragma once
#include<wrl.h>
#include <cassert>
#include <random>
#include<list>
#include "Allinclude/AllVector.h"
#include "../Math/RandomEngine.h"

struct Particle
{
	Vector3 position;//位置
	Vector3 scale;//スケール
	Vector3 rotate;//回転
	Vector3 velocity;//速度
	Vector4 color;//色
	int lifeTime;//寿命
	int currentTime;//経過時間
};



class ParticleEmitter
{
public:
	Particle MakeNewParticle(std::mt19937& randomEngine);

	void Update(int timer);

private:
	std::list<Particle>particles;

};

