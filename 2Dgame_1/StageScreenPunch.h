#pragma once
#include "raylib.h"
#include "Stage.h"
#include "ScreenPunchEffect.h"

void UpdateScreenPunchAreas(
    Stage& stage,
    ScreenPunchEffect& effect,
    const Rectangle& playerRect,
    float dt
);