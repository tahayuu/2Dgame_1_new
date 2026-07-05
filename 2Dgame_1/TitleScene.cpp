#include "TitleScene.h"

void TitleSceneInit(TitleScene& ts, Texture2D bg, Font& font, AudioManager& audio)
{
    ts.bg = bg;
    ts.font = &font;
    ts.audio = &audio;
}

// 更新：main.cpp の GameState::START ブロック（945〜993行）をここへ
bool TitleSceneUpdate(TitleScene& ts, GameState& gameState, float dt)
{
    if (!ts.isSelectingStage && IsKeyPressed(KEY_SPACE)) {
        ts.isSelectingStage = true;
        AudioPlaySfx(*ts.audio, SfxId::StageDecide);
    }

    if (ts.isSelectingStage) {
        bool moved = false;
        if (IsKeyPressed(KEY_UP)) {
            ts.selectStage = (ts.selectStage + ts.stageCount - 1) % ts.stageCount;
            moved = true;
        }
        if (IsKeyPressed(KEY_DOWN)) {
            ts.selectStage = (ts.selectStage + 1) % ts.stageCount;
            moved = true;
        }
        if (moved) {
            AudioPlaySfx(*ts.audio, SfxId::StageChoose);
        }

        // ENTER → ステージ開始
        if (IsKeyPressed(KEY_ENTER)) {
            AudioPlaySfx(*ts.audio, SfxId::StageDecide);
            ts.isSelectingStage = false;
            gameState = GameState::PLAYING;
            return true; // ← main に「LoadSelectedStage を呼んで」と伝える
        }
    }
    return false;
}

// 描画：main.cpp の START 描画ブロック（1059〜1085行）をここへ
void TitleSceneDraw(const TitleScene& ts, int screenWidth, int screenHeight)
{
    ClearBackground({ 20, 20, 40, 255 });

    if (ts.bg.id != 0) {
        DrawTexturePro(
            ts.bg,
            { 0, 0, (float)ts.bg.width, (float)ts.bg.height },
            { 0, 0, (float)screenWidth, (float)screenHeight },
            { 0, 0 }, 0.0f, WHITE);
    }

    if (!ts.isSelectingStage) {
        DrawTextEx(*ts.font, "PRESS SPACE",
            { screenWidth / 2.0f - 100, screenHeight / 2.0f + 20 }, 36, 0, YELLOW);
    }
    else {
        DrawTextEx(*ts.font,
            reinterpret_cast<const char*>(u8"モードせんたく"),
            { 100, 50 }, 48, 0, YELLOW);

        const char* stageLabels[] = {
            reinterpret_cast<const char*>(u8"ステージ１"),
            reinterpret_cast<const char*>(u8"デバッグモード"),
            reinterpret_cast<const char*>(u8"ステージ２"),
            reinterpret_cast<const char*>(u8"ステージ3"),
            reinterpret_cast<const char*>(u8"ステージせんたく"),
            reinterpret_cast<const char*>(u8"エディタステージ"),
        };
        for (int i = 0; i < ts.stageCount; i++) {
            Color c = (i == ts.selectStage) ? YELLOW : WHITE;
            DrawTextEx(*ts.font, stageLabels[i], { 150, 150.0f + i * 70 }, 36, 0, c);
        }
        DrawTextEx(*ts.font, "PRESS ENTER",
            { 150, 150.0f + ts.stageCount * 70 + 30 }, 24, 0, LIGHTGRAY);
    }
}

void TitleSceneUnload(TitleScene& ts)
{
    if (ts.bg.id != 0) UnloadTexture(ts.bg);
    ts.bg = {};
}