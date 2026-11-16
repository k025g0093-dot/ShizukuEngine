#pragma once
#include<wrl.h>
#include <cassert>
#include <random>
#include<list>
#include "Allinclude/AllVector.h"
#include "Math/RandomEngine.h"
#include"DeltaTaime/Clock.h"
#include "Math/AABB.h"
#include "ParticleTypes.h"


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

