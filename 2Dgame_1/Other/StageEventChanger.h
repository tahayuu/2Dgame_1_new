#pragma once

#include "StageTypes.h"
#include "raylib.h"

void UpdateEventChangers(
    Stage& stage,
    const Rectangle& playerRect
);

void ResetEventChangers(Stage& stage);