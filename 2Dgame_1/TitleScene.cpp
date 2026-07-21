#include "TitleScene.h"

// ================================================================
// 内部関数
// ================================================================

// 現在のタイトル画面に対応する背景を返す。
// title_2.png がまだ用意されていない場合は title_1.png を代わりに使う。
static Texture2D GetCurrentTitleBackground(const TitleScene& ts)
{
    if (ts.version == TitleVersion::NO_MORE_EASY && ts.title2Bg.id != 0) {
        return ts.title2Bg;
    }

    return ts.title1Bg;
}

// ================================================================
// 公開関数
// ================================================================

void TitleSceneInit(TitleScene& ts, Texture2D title1Bg, Texture2D title2Bg, Font& font, AudioManager& audio, bool allowAllStagesForDebug)
{
    ts.title1Bg = title1Bg;
    ts.title2Bg = title2Bg;
    ts.font = &font;
    ts.audio = &audio;
    ts.allowAllStagesForDebug = allowAllStagesForDebug;

    // ゲーム起動時は必ずタイトル画面1から始める。
    ts.version = TitleVersion::EASY_GAME;
    ts.selectStage = 0;
    ts.isSelectingStage = false;
}

int TitleSceneGetSelectableStageCount(const TitleScene& ts)
{
    // タイトル画面2では「ステージせんたく」だけを表示する。
    // Debug構成でも、この物語上の制限を優先する。
    if (ts.version == TitleVersion::NO_MORE_EASY) {
        return 1;
    }

    // タイトル画面1のDebug構成では、動作確認用に全項目を選べる。
    if (ts.allowAllStagesForDebug) {
        return TitleScene::STAGE_COUNT;
    }

    // Release構成のタイトル画面1ではステージ1だけを選べる。
    return 1;
}

void TitleSceneSetVersion(TitleScene& ts, TitleVersion version)
{
    ts.version = version;

    // タイトル画面2では、main.cpp の LoadSelectedStage にある
    // case 4（ステージ選択ステージ）を選択状態にする。
    if (version == TitleVersion::NO_MORE_EASY) {
        ts.selectStage = TitleScene::STAGE_SELECT_INDEX;
    }
    else {
        ts.selectStage = 0;
    }

    ts.isSelectingStage = false;
}

// 目的: タイトル入力を処理し、開始確定時に gameState を PLAYING へ進める。
// 出力: true のとき main 側で LoadSelectedStage を呼ぶ契機になる。
bool TitleSceneUpdate(TitleScene& ts, GameState& gameState, float dt)
{
    (void)dt;

    if (!ts.isSelectingStage && IsKeyPressed(KEY_SPACE)) {
        ts.isSelectingStage = true;
        AudioPlaySfx(*ts.audio, SfxId::StageDecide);
    }

    if (!ts.isSelectingStage) {
        return false;
    }

    // タイトル画面2では、上下入力による選択変更を行わず、
    // 「ステージせんたく」だけを確定できるようにする。
    if (ts.version == TitleVersion::NO_MORE_EASY) {
        ts.selectStage = TitleScene::STAGE_SELECT_INDEX;

        if (IsKeyPressed(KEY_ENTER)) {
            AudioPlaySfx(*ts.audio, SfxId::StageDecide);
            ts.isSelectingStage = false;
            gameState = GameState::PLAYING;
            return true;
        }

        return false;
    }

    const int selectableStageCount = TitleSceneGetSelectableStageCount(ts);
    bool moved = false;

    if (IsKeyPressed(KEY_UP)) {
        ts.selectStage = (ts.selectStage + selectableStageCount - 1) % selectableStageCount;
        moved = true;
    }

    if (IsKeyPressed(KEY_DOWN)) {
        ts.selectStage = (ts.selectStage + 1) % selectableStageCount;
        moved = true;
    }

    if (moved) {
        AudioPlaySfx(*ts.audio, SfxId::StageChoose);
    }

    // ENTER → 選択中のステージを開始
    if (IsKeyPressed(KEY_ENTER)) {
        AudioPlaySfx(*ts.audio, SfxId::StageDecide);
        ts.isSelectingStage = false;
        gameState = GameState::PLAYING;
        return true;
    }

    return false;
}

void TitleSceneDraw(const TitleScene& ts, int screenWidth, int screenHeight)
{
    ClearBackground({ 20, 20, 40, 255 });

    const Texture2D currentBg = GetCurrentTitleBackground(ts);
    if (currentBg.id != 0) {
        DrawTexturePro(
            currentBg,
            { 0, 0, (float)currentBg.width, (float)currentBg.height },
            { 0, 0, (float)screenWidth, (float)screenHeight },
            { 0, 0 },
            0.0f,
            WHITE
        );
    }

    if (!ts.isSelectingStage) {
        DrawTextEx(
            *ts.font,
            "PRESS SPACE",
            { screenWidth / 2.0f - 500, screenHeight / 2.0f + 20 },
            64,
            0,
            YELLOW
        );
        return;
    }

    const float modeSelectY = 270.0f;

    DrawTextEx(
        *ts.font,
        reinterpret_cast<const char*>(u8"モードせんたく"),
        { 100, modeSelectY },
        48,
        0,
        YELLOW
    );

    const char* stageLabels[TitleScene::STAGE_COUNT] = {
        reinterpret_cast<const char*>(u8"ステージ１"),
        reinterpret_cast<const char*>(u8"ステージ２"),
        reinterpret_cast<const char*>(u8"ステージ3"),
        reinterpret_cast<const char*>(u8"ステージ4"),
        reinterpret_cast<const char*>(u8"ステージせんたく"),
        reinterpret_cast<const char*>(u8"エディタステージ"),
    };

    const int selectableStageCount = TitleSceneGetSelectableStageCount(ts);
    const float stageListStartY = modeSelectY + 110.0f;

    // タイトル画面2は、ステージ選択ステージへの入口だけを表示する。
    if (ts.version == TitleVersion::NO_MORE_EASY) {
        DrawTextEx(
            *ts.font,
            stageLabels[TitleScene::STAGE_SELECT_INDEX],
            { 200, stageListStartY },
            36,
            0,
            YELLOW
        );

        DrawTextEx(
            *ts.font,
            "PRESS ENTER",
            { 200, stageListStartY + 100.0f },
            24,
            0,
            LIGHTGRAY
        );
        return;
    }

    for (int i = 0; i < selectableStageCount; i++) {
        const Color color = (i == ts.selectStage) ? YELLOW : WHITE;
        DrawTextEx(
            *ts.font,
            stageLabels[i],
            { 200, stageListStartY + i * 40.0f },
            36,
            0,
            color
        );
    }

    DrawTextEx(
        *ts.font,
        "PRESS ENTER",
        { 200, stageListStartY + selectableStageCount * 70.0f + 30.0f },
        24,
        0,
        LIGHTGRAY
    );
}

void TitleSceneUnload(TitleScene& ts)
{
    if (ts.title1Bg.id != 0) {
        UnloadTexture(ts.title1Bg);
    }

    if (ts.title2Bg.id != 0 && ts.title2Bg.id != ts.title1Bg.id) {
        UnloadTexture(ts.title2Bg);
    }

    ts.title1Bg = {};
    ts.title2Bg = {};
}