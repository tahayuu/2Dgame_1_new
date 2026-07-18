#include "StageEventChanger.h"
#include "StageGimmickSignal.h"

// 指定されたeventIdを持つDecoSpriteを探す
static DecoSprite* FindDecoSpriteByEventId(
    Stage& stage,
    int eventId
) {
    if (eventId <= 0) {
        return nullptr;
    }

    for (int i = 0;
        i < stage.decoSpriteCount;
        i++) {

        DecoSprite& deco =
            stage.decoSprites[i];

        if (deco.eventId == eventId) {
            return &deco;
        }
    }

    return nullptr;
}

// EventChangerを発動する
static void ApplyEventChanger(
    Stage& stage,
    EventChanger& event
) {
    DecoSprite* target =
        FindDecoSpriteByEventId(
            stage,
            event.targetEventId
        );

    // 最初に発動したときだけ元の状態を保存する
    if (!event.originalStateSaved) {
        if (target != nullptr) {
            event.originalSpriteId =
                target->spriteId;
        }

        event.originalJumpMode =
            stage.currentJumpMode;

        event.originalStateSaved = true;
    }

    // changedSpriteIdがNoneなら画像は変更しない
    if (target != nullptr &&
        event.changedSpriteId !=
        SpriteId::None) {

        target->spriteId =
            event.changedSpriteId;
    }

    // -1ならジャンプ設定は変更しない
    if (event.jumpMode >= 0) {
        stage.currentJumpMode =
            event.jumpMode;
    }

    event.triggered = true;
}

// EventChangerの変更を元へ戻す
static void RestoreEventChanger(
    Stage& stage,
    EventChanger& event
) {
    if (!event.originalStateSaved) {
        return;
    }

    DecoSprite* target =
        FindDecoSpriteByEventId(
            stage,
            event.targetEventId
        );

    if (target != nullptr) {
        target->spriteId =
            event.originalSpriteId;
    }

    // このイベントがジャンプモードを変更した場合のみ戻す
    if (event.jumpMode >= 0) {
        stage.currentJumpMode =
            event.originalJumpMode;
    }
}

void UpdateEventChangers(
    Stage& stage,
    const Rectangle& playerRect
) {
    for (int i = 0;
        i < stage.eventChangerCount;
        i++) {

        EventChanger& event =
            stage.eventChangers[i];

        // inputSignalIdが1以上なら信号型
        // 0なら従来のプレイヤー接触型
        const bool useSignal =
            event.inputSignalId > 0;

        bool active = false;
        bool wasActive = false;

        if (useSignal) {
            active =
                IsGimmickSignalActive(
                    stage,
                    event.inputSignalId
                );

            wasActive =
                event.signalWasActive;
        }
        else {
            active =
                CheckCollisionRecs(
                    playerRect,
                    event.rect
                );

            wasActive =
                event.playerWasInside;
        }

        // OFFからONになった瞬間
        const bool activated =
            active && !wasActive;

        // ONからOFFになった瞬間
        const bool deactivated =
            !active && wasActive;

        if (activated) {
            const bool canTrigger =
                !event.oneShot ||
                !event.triggered;

            if (canTrigger) {
                ApplyEventChanger(
                    stage,
                    event
                );
            }
        }

        if (deactivated &&
            event.restoreOnExit) {

            RestoreEventChanger(
                stage,
                event
            );
        }

        // 次のフレーム用に現在状態を保存する
        if (useSignal) {
            event.signalWasActive =
                active;
        }
        else {
            event.playerWasInside =
                active;
        }
    }
}

void ResetEventChangers(Stage& stage) {
    // EventChangerによって変更されたDecoSpriteを
    // Export直後の初期状態へ戻す
    for (int i = 0;
        i < stage.decoSpriteCount;
        i++) {

        stage.decoSprites[i] =
            stage.decoSpritesInit[i];
    }

    for (int i = 0;
        i < stage.eventChangerCount;
        i++) {

        EventChanger& event =
            stage.eventChangers[i];

        event.triggered = false;
        event.playerWasInside = false;
        event.signalWasActive = false;

        event.originalSpriteId =
            SpriteId::None;

        event.originalJumpMode = 0;
        event.originalStateSaved = false;
    }

    stage.currentJumpMode = 0;
}