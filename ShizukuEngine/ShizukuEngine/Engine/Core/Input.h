#pragma once
#include <windows.h>
#include <cmath> 
#include <cstring>

// ゲームパッド出力ができるように追加
#include <XInput.h>
#pragma comment(lib, "xinput.lib")

class Input {
public:
	void Update() {
		// --- キーボード ---
		memcpy(mKeysPrev, mKeysCurr, 256);
		GetKeyboardState(mKeysCurr);

		// --- マウス ---
		POINT point;
		GetCursorPos(&point);

		mMouseDeltaX = point.x - mMouseX;
		mMouseDeltaY = point.y - mMouseY;

		mMouseX = point.x;
		mMouseY = point.y;

		// --- ゲームパッド (追加) ---
		mPadPrev = mPadCurr; // 前フレームの状態を保存

		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		// 0番目のコントローラー（1P）の状態を取得
		if (XInputGetState(0, &state) == ERROR_SUCCESS) {
			mIsPadConnected = true;
			mPadCurr = state.Gamepad;
		}
		else {
			mIsPadConnected = false;
			ZeroMemory(&mPadCurr, sizeof(XINPUT_GAMEPAD));
		}
	}

	// --- キーボード関数 ---

	//キーの取得
	bool GetKey(int vKey)const { return (mKeysCurr[vKey] & 0x80) != 0; }
	//キーが押されたかを確認する関数
	bool GetKeyDown(int vKey)const { return ((mKeysCurr[vKey] & 0x80) != 0) && ((mKeysPrev[vKey] & 0x80) == 0); }
	//キーが離されたかを確認する関数
	bool GetKeyUp(int vKey) const { return ((mKeysCurr[vKey] & 0x80) == 0) && ((mKeysPrev[vKey] & 0x80) != 0); }

	// --- マウス関数 ---

	//マウスのX座標を取得する関数
	int GetMouseDeltaX()const { return mMouseDeltaX; }
	//マウスのY座標を取得する関数
	int GetMouseDeltaY() const { return mMouseDeltaY; }
	//マウスのボタンが押されているかを確認する関数（0:左ボタン、1:右ボタン、2:中ボタン）
	bool GetMouseButton(int button) {
		int vKey = (button == 0) ? VK_LBUTTON : (button == 1) ? VK_RBUTTON : VK_MBUTTON;
		return (mKeysCurr[vKey] & 0x80) != 0;
	}

	// --- ゲームパッド関数 (追加) ---

	// パッドが接続されているか確認
	bool IsPadConnected() const { return mIsPadConnected; }

	// ボタンを「押し続けているか」 (例: XINPUT_GAMEPAD_A)
	bool GetPadButton(WORD button)const {
		return (mPadCurr.wButtons & button) != 0;
	}

	// ボタンを「押した瞬間か」
	bool GetPadButtonDown(WORD button) const {
		return ((mPadCurr.wButtons & button) != 0) && ((mPadPrev.wButtons & button) == 0);
	}

	// ボタンを「離した瞬間か」
	bool GetPadButtonUp(WORD button)const {
		return ((mPadCurr.wButtons & button) == 0) && ((mPadPrev.wButtons & button) != 0);
	}

	// 左スティックの入力を取る（戻り値は -1.0f ～ 1.0f。遊びも自動計算）
	float GetLeftStickX()const {
		if (std::abs(mPadCurr.sThumbLX) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) return 0.0f;
		return mPadCurr.sThumbLX / 32767.0f;
	}
	float GetLeftStickY() const {
		if (std::abs(mPadCurr.sThumbLY) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) return 0.0f;
		return mPadCurr.sThumbLY / 32767.0f;
	}

	float GetLeftStick() const {
		if (std::abs(mPadCurr.sThumbLX) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) return 0.0f;
		return mPadCurr.sThumbLX / 32767.0f;
	}

	// 右スティックの入力を取る（戻り値は -1.0f ～ 1.0f。遊びも自動計算）
	float GetRightStickX() const {
		if (std::abs(mPadCurr.sThumbRX) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) return 0.0f;
		return mPadCurr.sThumbRX / 32767.0f;
	}
	float GetRightStickY() const {
		if (std::abs(mPadCurr.sThumbRY) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) return 0.0f;
		return mPadCurr.sThumbRY / 32767.0f;
	}

	float GetRightStick() const {
		if (std::abs(mPadCurr.sThumbRX) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) return 0.0f;
		return mPadCurr.sThumbRX / 32767.0f;
	}

private:
	// キーボード管理
	BYTE mKeysCurr[256] = { 0 };
	BYTE mKeysPrev[256] = { 0 };

	// マウス管理
	int mMouseX = 0;
	int mMouseY = 0;
	int mMouseDeltaX = 0;
	int mMouseDeltaY = 0;

	// ゲームパッド管理 (追加)
	bool mIsPadConnected = false;
	XINPUT_GAMEPAD mPadCurr = { 0 };
	XINPUT_GAMEPAD mPadPrev = { 0 };
};