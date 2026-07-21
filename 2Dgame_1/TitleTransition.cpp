#include "TitleTransition.h"
#include <algorithm>

namespace {
    float Clamp01(float value)
    {
        return std::clamp(value, 0.0f, 1.0f);
    }

    unsigned char ToAlphaByte(float value)
    {
        return static_cast<unsigned char>(Clamp01(value) * 255.0f);
    }
}

void TitleTransitionBegin(TitleTransition& transition)
{
    transition.phase = TitleTransitionPhase::FADE_OUT_DIALOG;
    transition.timer = 0.0f;
}

TitleTransitionResult TitleTransitionUpdate(TitleTransition& transition, float dt)
{
    switch (transition.phase) {
    case TitleTransitionPhase::FADE_OUT_DIALOG:
        transition.timer += dt;

        if (transition.timer >= TitleTransition::FADE_OUT_SECONDS) {
            transition.phase = TitleTransitionPhase::FADE_IN_TITLE;
            transition.timer = 0.0f;
            return TitleTransitionResult::SWITCH_TO_TITLE;
        }
        break;

    case TitleTransitionPhase::FADE_IN_TITLE:
        transition.timer += dt;

        if (transition.timer >= TitleTransition::FADE_IN_SECONDS) {
            transition.phase = TitleTransitionPhase::INACTIVE;
            transition.timer = 0.0f;
            return TitleTransitionResult::FINISHED;
        }
        break;

    case TitleTransitionPhase::INACTIVE:
        break;
    }

    return TitleTransitionResult::NONE;
}

bool TitleTransitionIsActive(const TitleTransition& transition)
{
    return transition.phase != TitleTransitionPhase::INACTIVE;
}

bool TitleTransitionShouldDrawTitle(const TitleTransition& transition)
{
    return transition.phase == TitleTransitionPhase::FADE_IN_TITLE;
}

void TitleTransitionDrawOverlay(const TitleTransition& transition, int screenWidth, int screenHeight)
{
    float blackAlpha = 0.0f;

    if (transition.phase == TitleTransitionPhase::FADE_OUT_DIALOG) {
        blackAlpha = transition.timer / TitleTransition::FADE_OUT_SECONDS;
    }
    else if (transition.phase == TitleTransitionPhase::FADE_IN_TITLE) {
        blackAlpha = 1.0f - transition.timer / TitleTransition::FADE_IN_SECONDS;
    }
    else {
        return;
    }

    DrawRectangle(
        0,
        0,
        screenWidth,
        screenHeight,
        { 0, 0, 0, ToAlphaByte(blackAlpha) }
    );
}