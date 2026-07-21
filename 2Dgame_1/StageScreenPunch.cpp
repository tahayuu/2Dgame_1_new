#include "StageScreenPunch.h"

void UpdateScreenPunchAreas(
    Stage& stage,
    ScreenPunchEffect& effect,
    const Rectangle& playerRect,
    float dt
) {
    if (effect.phase == ScreenPunchPhase::APPROACH) {
        const int areaIndex = effect.activeAreaIndex;

        if (areaIndex < 0 || areaIndex >= stage.ojisanPunchAreaCount) {
            ScreenPunchEffectCancel(effect);
            return;
        }

        const Rectangle& area = stage.ojisanPunchAreas[areaIndex];
        const bool playerInside = CheckCollisionRecs(playerRect, area);

        // 逃げ切った
        if (!playerInside) {
            stage.ojisanPunchTriggered[areaIndex] = false;
            ScreenPunchEffectCancel(effect);
            return;
        }

        ScreenPunchEffectUpdate(effect, dt);
        return;
    }

    if (effect.phase == ScreenPunchPhase::IMPACT) {
        ScreenPunchEffectUpdate(effect, dt);
        return;
    }

    if (effect.phase == ScreenPunchPhase::STUCK) {
        return;
    }

    for (int i = 0; i < stage.ojisanPunchAreaCount; i++) {
        const bool playerInside =
            CheckCollisionRecs(playerRect, stage.ojisanPunchAreas[i]);

        if (!playerInside) {
            stage.ojisanPunchTriggered[i] = false;
            continue;
        }

        if (stage.ojisanPunchTriggered[i]) {
            continue;
        }

        stage.ojisanPunchTriggered[i] = true;
        ScreenPunchEffectStart(
            effect,
            i,
            stage.ojisanPunchDrawFront[i],
            stage.ojisanPunchApproachDuration[i]
        );
        return;
    }
}