#pragma once
#include <chrono>

//デルタタイマー管理クラス

class Clock
{

public:

	void Update();

	float GetDeltaTime() const { return mDeltaTime; }

private:
	std::chrono::high_resolution_clock::time_point mPrevTime;
	float mDeltaTime = 0.0f;
	bool mIsFistFrame = 0.0f;
};

