#include "ScreenPunchEffect.h"
#include <cmath>

/* 経過時間から進行率を求める
        ↓
Clamp01で0～1に収める
        ↓
EaseInCubicで加速する形に変える
        ↓
LerpFloatで拳の大きさを決める*/

//タイマーが少し超えることがあるので進捗率を0～1に収める関数
static float Clamp01(float value) {
	if (value < 0.0f) return 0.0f;
	if (value > 1.0f) return 1.0f;
	return value;
}

//開始値から終了値までの途中の数値を求める関数
//拳の大きさを決める
static float LerpFloat(float start, float end, float t) {
	return start + (end - start) * t;
}
//最初はゆっくり、後半になるほど速く変化する
/*最初：小さく、変化も少ない
中盤：少しずつ大きくなる
最後：一気に画面全体を覆う

という変化のほうが自然*/
static float EaseInCubic(float t) {
    return t * t * t;
}

// 最初に大きく動き、最後にゆっくり止まる。
// プレイヤーが画面へ吹き飛んで張り付く動きに使う。
static float EaseOutCubic(float t) {
    const float inv = 1.0f - t;
    return 1.0f - inv * inv * inv;
}

void ScreenPunchEffectInit(ScreenPunchEffect& effect) {
    effect.fistTexture =
        LoadTexture("assets/images/effects/fist_front.png");

    effect.impactTexture =
        LoadTexture("assets/images/effects/punch_impact.png");

    effect.stuckPlayerTexture =
        LoadTexture("assets/images/effects/player_stuck_screen2.png");

    ScreenPunchEffectReset(effect);
}

void ScreenPunchEffectUnload(ScreenPunchEffect& effect) {
    if (effect.fistTexture.id != 0) UnloadTexture(effect.fistTexture);
    if (effect.impactTexture.id != 0) UnloadTexture(effect.impactTexture);
    if (effect.stuckPlayerTexture.id != 0) UnloadTexture(effect.stuckPlayerTexture);
}

// こぶしエフェクトを開始する
void ScreenPunchEffectStart(
    ScreenPunchEffect& effect,
    int areaIndex,
    bool drawFistInFront,
    float approachDuration
) {
    effect.phase = ScreenPunchPhase::APPROACH;
    effect.timer = 0.0f;
    effect.activeAreaIndex = areaIndex;
    effect.drawFistInFront = drawFistInFront;
    effect.approachDuration =
        (approachDuration > 0.0f) ? approachDuration : 0.85f;
    effect.killRequested = false;

    effect.showStuckPlayer = false;
    effect.stuckPlayerStartScreenPos = { 0.0f, 0.0f };
    effect.stuckPlayerTargetScreenPos = { 0.0f, 0.0f };
}
// こぶしエフェクトをキャンセルする
void ScreenPunchEffectCancel(ScreenPunchEffect& effect) {
    effect.phase = ScreenPunchPhase::IDLE;
    effect.timer = 0.0f;
    effect.activeAreaIndex = -1;
    effect.drawFistInFront = false;
    effect.killRequested = false;

    effect.showStuckPlayer = false;
    effect.stuckPlayerStartScreenPos = { 0.0f, 0.0f };
    effect.stuckPlayerTargetScreenPos = { 0.0f, 0.0f };
}

void ScreenPunchEffectReset(ScreenPunchEffect& effect) {
    ScreenPunchEffectCancel(effect);
}


void ScreenPunchEffectSetStuckPlayer(
    ScreenPunchEffect& effect,
    Vector2 startScreenPos,
    Vector2 targetScreenPos
) {
    effect.showStuckPlayer = true;
    effect.stuckPlayerStartScreenPos = startScreenPos;
    effect.stuckPlayerTargetScreenPos = targetScreenPos;
}

void ScreenPunchEffectUpdate(ScreenPunchEffect& effect, float dt) {
    if (effect.phase == ScreenPunchPhase::IDLE) {
        return;
    }

    effect.timer += dt;

    if (effect.phase == ScreenPunchPhase::APPROACH) {
        if (effect.timer >= effect.approachDuration) {
            effect.phase = ScreenPunchPhase::IMPACT;
            effect.timer = 0.0f;
            effect.killRequested = true;
        }
        return;
    }

    if (effect.phase == ScreenPunchPhase::IMPACT) {
        if (effect.timer >= effect.impactDuration) {
            effect.phase = ScreenPunchPhase::STUCK;
            effect.timer = 0.0f;
        }
        return;
    }

    // 張り付き中はリスタートされるまで状態を維持する。
    if (effect.phase == ScreenPunchPhase::STUCK) {
        return;
    }
}
//この関数に、パンチ演出の情報画面の横幅画面の縦幅拳をどのくらいの大きさで表示するかを渡すと、画面中央に拳を描画します。
static void DrawFist(
    const ScreenPunchEffect& effect,
    int screenWidth,
    int screenHeight,
    float widthRatio,
    Color tint
) {
    if (effect.fistTexture.id == 0)return;

	//拳の描画サイズを決める
	const float drawWidth = screenWidth * widthRatio;
	const float aspect = (float)effect.fistTexture.height / (float)effect.fistTexture.width;//アスペクト比を求める
	const float drawHeight = drawWidth * aspect;//アスペクト比を維持して高さを決める

	//拳の描画位置を決める
    Rectangle source = {
    0.0f,
    0.0f,
    (float)effect.fistTexture.width,
    (float)effect.fistTexture.height
    };

    //画面上のどこに、どの大きさで描画するか
    Rectangle destination = {
    screenWidth * 0.5f,
    screenHeight * 0.5f,
    drawWidth,
    drawHeight
    };

    Vector2 origin = {
    drawWidth * 0.5f,
    drawHeight * 0.5f
    };

    DrawTexturePro(
        effect.fistTexture,
        source,//元画像の使用範囲
        destination,//画面上の位置と大きさ
        origin,//描画の基準点
        0.0f,//回転しない
        tint
    );
 
}

static void DrawStuckPlayer(
    const ScreenPunchEffect& effect,
    float moveProgress
) {
    if (!effect.showStuckPlayer) return;
    if (effect.stuckPlayerTexture.id == 0) return;

    const float t = EaseOutCubic(Clamp01(moveProgress));

    const Vector2 currentPos = {
        LerpFloat(
            effect.stuckPlayerStartScreenPos.x,
            effect.stuckPlayerTargetScreenPos.x,
            t
        ),
        LerpFloat(
            effect.stuckPlayerStartScreenPos.y,
            effect.stuckPlayerTargetScreenPos.y,
            t
        )
    };

    const float currentScale = LerpFloat(
        effect.stuckPlayerStartScale,
        effect.stuckPlayerScale,
        t
    );

    const float drawWidth =
        effect.stuckPlayerTexture.width * currentScale;

    const float drawHeight =
        effect.stuckPlayerTexture.height * currentScale;

    Rectangle source = {
        0.0f,
        0.0f,
        (float)effect.stuckPlayerTexture.width,
        (float)effect.stuckPlayerTexture.height
    };

    Rectangle destination = {
        currentPos.x,
        currentPos.y,
        drawWidth,
        drawHeight
    };

    Vector2 origin = {
        drawWidth * 0.5f,
        drawHeight * 0.5f
    };

    DrawTexturePro(
        effect.stuckPlayerTexture,
        source,
        destination,
        origin,
        0.0f,
        WHITE
    );
}

void ScreenPunchEffectDrawFistLayer(
    const ScreenPunchEffect& effect,
    int screenWidth,
    int screenHeight
) {
    if (effect.phase == ScreenPunchPhase::IDLE ||
        effect.phase == ScreenPunchPhase::STUCK) {
        return;
    }

    if (effect.phase == ScreenPunchPhase::APPROACH) {
        const float duration = (effect.approachDuration > 0.0f)
            ? effect.approachDuration
            : 0.01f;

        const float t = Clamp01(effect.timer / duration);
        const float easedT = EaseInCubic(t);
        const float widthRatio = LerpFloat(
            effect.startWidthRatio,
            effect.endWidthRatio,
            easedT
        );

        DrawFist(effect, screenWidth, screenHeight, widthRatio, WHITE);
        return;
    }

    if (effect.phase == ScreenPunchPhase::IMPACT) {
        const float duration = (effect.impactDuration > 0.0f)
            ? effect.impactDuration
            : 0.01f;

        const float t = Clamp01(effect.timer / duration);
        const float alpha = 1.0f - t;

        DrawFist(
            effect,
            screenWidth,
            screenHeight,
            effect.endWidthRatio,
            ColorAlpha(WHITE, alpha)
        );
    }
}

void ScreenPunchEffectDrawOverlay(
    const ScreenPunchEffect& effect,
    int screenWidth,
    int screenHeight
) {
    if (effect.phase == ScreenPunchPhase::IDLE) {
        return;
    }

    if (effect.phase == ScreenPunchPhase::APPROACH) {
        const float duration = (effect.approachDuration > 0.0f)
            ? effect.approachDuration
            : 0.01f;

        const float t = Clamp01(effect.timer / duration);
        DrawRectangle(
            0,
            0,
            screenWidth,
            screenHeight,
            ColorAlpha(BLACK, t * 0.15f)
        );
        return;
    }

    if (effect.phase == ScreenPunchPhase::IMPACT) {
        const float duration = (effect.impactDuration > 0.0f)
            ? effect.impactDuration
            : 0.01f;

        const float t = Clamp01(effect.timer / duration);
        const float alpha = 1.0f - t;

        DrawRectangle(
            0,
            0,
            screenWidth,
            screenHeight,
            ColorAlpha(WHITE, alpha * 0.45f)
        );

        if (effect.impactTexture.id != 0) {
            Rectangle source = {
                0.0f,
                0.0f,
                (float)effect.impactTexture.width,
                (float)effect.impactTexture.height
            };

            Rectangle destination = {
                0.0f,
                0.0f,
                (float)screenWidth,
                (float)screenHeight
            };

            DrawTexturePro(
                effect.impactTexture,
                source,
                destination,
                { 0.0f, 0.0f },
                0.0f,
                ColorAlpha(WHITE, alpha)
            );
        }

        DrawStuckPlayer(effect, t);
        return;
    }

    if (effect.phase == ScreenPunchPhase::STUCK) {
        DrawStuckPlayer(effect, 1.0f);
    }
}

bool ScreenPunchEffectShouldDrawFistInFront(
    const ScreenPunchEffect& effect
) {
    return effect.drawFistInFront;
}

void ScreenPunchEffectDraw(
    const ScreenPunchEffect& effect,
    int screenWidth,
    int screenHeight
) {
    ScreenPunchEffectDrawFistLayer(effect, screenWidth, screenHeight);
    ScreenPunchEffectDrawOverlay(effect, screenWidth, screenHeight);
}

//パンチ演出が現在動いているか調べる
bool ScreenPunchEffectIsActive(const ScreenPunchEffect& effect) {
    return effect.phase != ScreenPunchPhase::IDLE;
}

//死亡要求を1回だけ受け取る
bool ScreenPunchEffectConsumeKillRequest(ScreenPunchEffect& effect) {
    if (!effect.killRequested) {
        return false;
    }

    effect.killRequested = false;
    return true;
}

Vector2 ScreenPunchEffectGetCameraShake(
    const ScreenPunchEffect& effect
) {
    if (effect.phase != ScreenPunchPhase::IMPACT) {
        return { 0.0f, 0.0f };
    }

    float t = Clamp01(effect.timer / effect.impactDuration);
    float strength = 16.0f * (1.0f - t);

    return {
        GetRandomValue(-100, 100) / 100.0f * strength,
        GetRandomValue(-100, 100) / 100.0f * strength
    };
}