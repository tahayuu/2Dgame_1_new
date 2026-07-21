#pragma once
#include "raylib.h"

enum class ScreenPunchPhase {
	IDLE,
	APPROACH,
	IMPACT,
	STUCK,
};

struct ScreenPunchEffect {
	Texture2D fistTexture{};
	Texture2D impactTexture{};
	Texture2D stuckPlayerTexture{};

	ScreenPunchPhase phase = ScreenPunchPhase::IDLE;

	float timer = 0.0f;

	float approachDuration = 0.85f;//こぶしが画面に近づく時間 

	float impactDuration = 0.40f;//こぶしが画面に衝突している時間
	
	float startWidthRatio = 0.08f; 
	float endWidthRatio = 1.60f;

	int activeAreaIndex = -1;
	bool drawFistInFront = false;
	bool killRequested = false;

	//張り付き画像用
	bool showStuckPlayer = false;
	Vector2 stuckPlayerStartScreenPos{ 0.0f, 0.0f };
	Vector2 stuckPlayerTargetScreenPos{ 0.0f, 0.0f };
	float stuckPlayerStartScale = 0.35f;
	float stuckPlayerScale = 1.0f;
};


void ScreenPunchEffectInit(ScreenPunchEffect& effect);
void ScreenPunchEffectUnload(ScreenPunchEffect& effect);

void ScreenPunchEffectStart(
	ScreenPunchEffect& effect,
	int areaIndex,
	bool drawFistInFront,
	float approachDuration
);
void ScreenPunchEffectCancel(ScreenPunchEffect& effect);
void ScreenPunchEffectReset(ScreenPunchEffect& effect);
void ScreenPunchEffectUpdate(ScreenPunchEffect& effect, float dt);

void ScreenPunchEffectSetStuckPlayer(
	ScreenPunchEffect& effect,
	Vector2 startScreenPos,
	Vector2 targetScreenPos
);

// 拳本体だけを描画する。main.cpp側で前景/背景の位置を選んで呼ぶ。
void ScreenPunchEffectDrawFistLayer(
	const ScreenPunchEffect& effect,
	int screenWidth,
	int screenHeight
);

// 暗転・白フラッシュ・衝撃波・張り付きプレイヤーを最前面に描画する。
void ScreenPunchEffectDrawOverlay(
	const ScreenPunchEffect& effect,
	int screenWidth,
	int screenHeight
);

bool ScreenPunchEffectShouldDrawFistInFront(const ScreenPunchEffect& effect);

// 互換用。拳とオーバーレイをまとめて描画する。
void ScreenPunchEffectDraw(
	const ScreenPunchEffect& effect,
	int screenWidth,
	int screenHeight
);

bool ScreenPunchEffectIsActive(const ScreenPunchEffect& effect);
bool ScreenPunchEffectConsumeKillRequest(ScreenPunchEffect& effect);

Vector2 ScreenPunchEffectGetCameraShake(
	const ScreenPunchEffect& effect
);


