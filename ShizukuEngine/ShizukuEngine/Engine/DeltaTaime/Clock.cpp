#include "Clock.h"
#include <Windows.h>
#pragma comment(lib, "winmm.lib")

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

void Clock::InitializeFixFPS() {
	mReference = std::chrono::steady_clock::now();
	timeBeginPeriod(1);
}

void Clock::UpdateFixFPS() {

	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));

	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	std::chrono::microseconds elapsed =
		std::chrono::duration_cast<std::chrono::microseconds>(now - mReference);

	if (elapsed < kMinCheckTime) {

		while (std::chrono::steady_clock::now() - mReference < kMinTime)
		{

			std::this_thread::sleep_for(std::chrono::microseconds(1));
			std::this_thread::yield();

		}
	}
	mReference = std::chrono::steady_clock::now();
}