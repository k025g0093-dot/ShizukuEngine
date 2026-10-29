#include "ParticleEmitter.h"

Particle ParticleEmitter::MakeNewParticle(std::mt19937& randomEngine) {

	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	Particle particle;
	particle.scale = { 1.0f,1.0f,1.0f };
	particle.rotate = { 0.0f,0.0f,0.0f };
	particle.position = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
	particle.velocity = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
	return particle;
}

void ParticleEmitter::Update(int timer) {

	for (std::list<Particle>::iterator particleIterator = particles.begin();
		particleIterator!=particles.end();
		) {

		//寿命が尽きたパーティクルは更新しない
		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			particleIterator = particles.erase(particleIterator);
			continue;
		}



		++particleIterator;
	}

}
