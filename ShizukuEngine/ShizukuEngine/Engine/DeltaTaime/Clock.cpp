#include "Clock.h"

void Clock::Update() {

	auto currentTime = std::chrono::high_resolution_clock::now();
	if (mIsFistFrame) {
		mPrevTime = currentTime;
		mDeltaTime = 0.0f;
		mIsFistFrame = false;
		return;
	}
	std::chrono::duration<float> elapsed = currentTime - mPrevTime;
	mDeltaTime = elapsed.count();
	mPrevTime = currentTime;
}
