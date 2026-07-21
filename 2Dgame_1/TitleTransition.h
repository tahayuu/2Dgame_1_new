#pragma once
#include "raylib.h"

// ================================================================
// TitleTransition.h の役割
// ---------------------------------------------------------------
// ・ステージクリア後の会話画面からタイトル画面2へ戻る際の
//   フェードアウト / フェードインを管理する。
// ・main.cppには「どちらの画面を描くか」と「切替タイミング」だけを返す。
// ================================================================

enum class TitleTransitionPhase {
    INACTIVE,
    FADE_OUT_DIALOG,
    FADE_IN_TITLE
};

enum class TitleTransitionResult {
    NONE,
    SWITCH_TO_TITLE,
    FINISHED
};

struct TitleTransition {
    static constexpr float FADE_OUT_SECONDS = 0.85f;
    static constexpr float FADE_IN_SECONDS = 0.85f;

    TitleTransitionPhase phase = TitleTransitionPhase::INACTIVE;
    float timer = 0.0f;
};

void TitleTransitionBegin(TitleTransition& transition);
TitleTransitionResult TitleTransitionUpdate(TitleTransition& transition, float dt);
bool TitleTransitionIsActive(const TitleTransition& transition);
bool TitleTransitionShouldDrawTitle(const TitleTransition& transition);
void TitleTransitionDrawOverlay(const TitleTransition& transition, int screenWidth, int screenHeight);