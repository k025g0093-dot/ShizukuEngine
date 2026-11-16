#pragma once
#include <chrono>
#include <thread>
//デルタタイマー管理クラス

class Clock
{

public:

	void Update();

	float GetDeltaTime() const { return mDeltaTime; }

	void InitializeFixFPS();

	void UpdateFixFPS();

private:
	std::chrono::high_resolution_clock::time_point mPrevTime;
	std::chrono::high_resolution_clock::time_point mReference;
	float mDeltaTime = 0.0f;
	bool mIsFistFrame = 0.0f;
};

