#include "ParticleEmitter.h"


ParticleEmitter::ParticleEmitter() {
	std::random_device randomDevice{};
	mRandomEngine.seed(randomDevice());
}

ParticleEmitter::~ParticleEmitter() {}

Particle ParticleEmitter::MakeNewParticle(
	std::mt19937& randomEngine,
	const Emitter& emitter) {

	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);

	Particle particle;
	particle.scale = { 1.0f,1.0f,1.0f };
	particle.rotate = { 0.0f,0.0f,0.0f };
	particle.position = {
		emitter.position.x+distribution(randomEngine),
		emitter.position.y + distribution(randomEngine) ,
		emitter.position.z + distribution(randomEngine)
	};

	particle.velocity = {
		distribution(randomEngine) * emitter.velocityRange.x,
		distribution(randomEngine) * emitter.velocityRange.y ,
		distribution(randomEngine) * emitter.velocityRange.z
	};

	particle.color = {
		distColor(randomEngine),
		distColor(randomEngine),
		distColor(randomEngine),
		1.0f
	};

	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;

	return particle;
}

void ParticleEmitter::Update(float timer) {

	//----- 発生 -----

	mEmitter.frequencyTime += timer;
	if (mEmitter.frequency <= mEmitter.frequencyTime) {

		for (uint32_t count = 0; count < mEmitter.count; ++count) {
			particles.push_back(
				MakeNewParticle(mRandomEngine, mEmitter)
			);
		}
		mEmitter.frequencyTime -= mEmitter.frequency;
	}

	//----- 更新 -----
	for (std::list<Particle>::iterator particleIterator = particles.begin();
		particleIterator != particles.end();
		) {
		//寿命が尽きたパーティクルは更新しない
		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			particleIterator = particles.erase(particleIterator);
			continue;
		}

		particleIterator->position += particleIterator->velocity*timer;

		particleIterator->currentTime += timer;

		particleIterator->color.w=1.0f- (particleIterator->currentTime / particleIterator->lifeTime);
		++particleIterator;
	}
}

void ParticleEmitter::CreateEmitter(const Emitter& emitter) {
	mEmitter = emitter;
	for (uint32_t count = 0; count < mEmitter.count; ++count) {
		particles.push_back(
			MakeNewParticle(mRandomEngine, mEmitter)
		);
	}
}
