#include "TitleScene.h"

// TitleScene.cpp の役割: タイトル入力と選択UIの描画を実装する。
void TitleSceneInit(TitleScene& ts, Texture2D bg, Font& font, AudioManager& audio)
{
    ts.bg = bg;
    ts.font = &font;
    ts.audio = &audio;
}

// 目的: タイトル入力を処理し、開始確定時に gameState を PLAYING へ進める。
// 出力: true のとき main 側で LoadSelectedStage を呼ぶ契機になる。
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
        // PRESS SPACE：文字サイズを大きくして左側にずらす
        DrawTextEx(*ts.font, "PRESS SPACE",
            { screenWidth / 2.0f - 500, screenHeight / 2.0f + 20 }, 64, 0, YELLOW);
    }
    else {
        // モードせんたくの表示位置を下にずらす（連動して以下の項目もずれる）
        const float modeSelectY = 270.0f;

        DrawTextEx(*ts.font,
            reinterpret_cast<const char*>(u8"モードせんたく"),
            { 100, modeSelectY }, 48, 0, YELLOW);

        const char* stageLabels[] = {
            reinterpret_cast<const char*>(u8"ステージ１"),
            reinterpret_cast<const char*>(u8"デバッグモード"),
            reinterpret_cast<const char*>(u8"ステージ２"),
            reinterpret_cast<const char*>(u8"ステージ3"),
            reinterpret_cast<const char*>(u8"ステージせんたく"),
            reinterpret_cast<const char*>(u8"エディタステージ"),
        };
        // ステージ項目もモードせんたくのY座標に合わせてずらす
        const float stageListStartY = modeSelectY + 110.0f;
        for (int i = 0; i < ts.stageCount; i++) {
            Color c = (i == ts.selectStage) ? YELLOW : WHITE;
			DrawTextEx(*ts.font, stageLabels[i], { 200, stageListStartY + i * 40 }, 36, 0, c);// 70px 間隔で縦に並べる
        }
        DrawTextEx(*ts.font, "PRESS ENTER",
            { 200, stageListStartY + ts.stageCount * 70 + 30 }, 24, 0, LIGHTGRAY);
    }
}

void TitleSceneUnload(TitleScene& ts)
{
    if (ts.bg.id != 0) UnloadTexture(ts.bg);
    ts.bg = {};
}