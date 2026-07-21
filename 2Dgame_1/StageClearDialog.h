#pragma once
#include "raylib.h"
#include "AudioManager.h"
#include <array>
#include <cstddef>
#include <string>

// ================================================================
// StageClearDialog.h の役割
// ---------------------------------------------------------------
// ・各ステージのクリア後に共通で使用する会話画面を管理する。
// ・クリアしたステージ番号に応じて、会話文とおじさん画像を切り替える。
// ・停止したステージ画面 → 暗転 → おじさん登場 → 文字送りを行う。
// ・ENTERで全文表示／次のセリフへ進む。
// ・ステージ1は最後のTABでポーズ画面を開く。
// ・ステージ2以降は最後のセリフ表示後、ENTERでステージ選択へ進む。
// ================================================================

enum class StageClearDialogResult {
    NONE,
    OPEN_PAUSE,
    GO_TO_STAGE_SELECT
};

enum class StageClearDialogPhase {
    WAIT_AFTER_CLEAR,   // クリア直後のステージ画面を少し残す
    FADE_TO_BLACK,      // ステージ画面を黒で覆っていく
    PORTRAIT_FADE_IN,   // 黒背景の上へおじさんを出現させる
    TYPING,             // セリフを1文字ずつ表示する
    READY               // 全文表示済み。次の入力を待つ
};

struct StageClearDialog {
    static constexpr int MAX_LINE_COUNT = 10;

    static constexpr float WAIT_AFTER_CLEAR_SECONDS = 0.80f;
    static constexpr float FADE_TO_BLACK_SECONDS = 0.90f;
    static constexpr float PORTRAIT_FADE_IN_SECONDS = 1.15f;
    static constexpr float CHARACTER_INTERVAL_SECONDS = 0.055f;
    static constexpr float TALK_SE_INTERVAL_SECONDS = 0.070f;

    // stageN_clear_M のキーから読み込んだ会話文。
    std::array<std::string, MAX_LINE_COUNT> lines{};

    // assets/images/ojisan/stageN_clear/dialog_MM.png から読み込む。
    // ファイルがない行は fallbackPortrait を使用する。
    std::array<Texture2D, MAX_LINE_COUNT> linePortraits{};

    const Texture2D* fallbackPortrait = nullptr;
    const Font* font = nullptr;
    AudioManager* audio = nullptr;

    int clearedStage = 0;
    int lineCount = 0;
    int lineIndex = 0;
    StageClearDialogPhase phase = StageClearDialogPhase::WAIT_AFTER_CLEAR;

    float phaseTimer = 0.0f;
    float characterTimer = 0.0f;
    float talkSeTimer = 0.0f;

    // UTF-8文字列の先頭から何バイトまで表示するか。
    std::size_t visibleByteCount = 0;

    bool active = false;
};

void StageClearDialogInit(StageClearDialog& dialog, const Texture2D& fallbackPortrait, const Font& font, AudioManager& audio);
void StageClearDialogUnload(StageClearDialog& dialog);
void StageClearDialogBegin(StageClearDialog& dialog, int clearedStage);
void StageClearDialogEnd(StageClearDialog& dialog);
StageClearDialogResult StageClearDialogUpdate(StageClearDialog& dialog, float dt);

bool StageClearDialogShouldDrawStageBehind(const StageClearDialog& dialog);
void StageClearDialogDraw(const StageClearDialog& dialog, int screenWidth, int screenHeight);
