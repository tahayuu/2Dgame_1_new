#pragma once
#include "raylib.h"
#include "AudioManager.h"
#include <cstddef>

// ================================================================
// Stage1ClearDialog.h の役割
// ---------------------------------------------------------------
// ・ステージ1クリア後に表示する専用会話画面の状態を保持する。
// ・ステージ画面を少し残した後、黒画面へフェードする。
// ・おじさん画像をフェードインさせる。
// ・日本語をUTF-8の1文字単位で順番に表示する。
// ・ENTERによる全文表示と、次のセリフへの移動を管理する。
// ・最後のTABでは直接タイトルへ戻さず、main.cppへポーズ要求を返す。
// ・セリフごとに上半分のおじさん画像を切り替える。
// ================================================================

enum class Stage1ClearDialogResult {
    NONE,
    OPEN_PAUSE
};

// 会話演出が現在どの段階にいるかを表す。
enum class Stage1ClearDialogPhase {
    WAIT_AFTER_CLEAR,   // ステージ1の画面を少しだけ残す
    FADE_TO_BLACK,      // ステージ画面を黒で覆っていく
    PORTRAIT_FADE_IN,   // 黒背景の上へおじさんを出現させる
    TYPING,             // セリフを1文字ずつ表示する
    READY               // 全文表示済み。次の入力を待つ
};

struct Stage1ClearDialog {
    static constexpr int LINE_COUNT = 6;

    // 演出速度。後からここだけ変更すればテンポを調整できる。
    static constexpr float WAIT_AFTER_CLEAR_SECONDS = 0.80f;
    static constexpr float FADE_TO_BLACK_SECONDS = 0.90f;
    static constexpr float PORTRAIT_FADE_IN_SECONDS = 1.15f;
    static constexpr float CHARACTER_INTERVAL_SECONDS = 0.055f;

    // 同じ会話SEを短時間に重ねすぎないための最小間隔。
    static constexpr float TALK_SE_INTERVAL_SECONDS = 0.070f;

    // 各セリフ専用の画像。画像が存在しない行はfallbackPortraitを使用する。
    Texture2D linePortraits[LINE_COUNT]{};
    const Texture2D* fallbackPortrait = nullptr;

    const Font* font = nullptr;
    AudioManager* audio = nullptr;

    int lineIndex = 0;
    Stage1ClearDialogPhase phase = Stage1ClearDialogPhase::WAIT_AFTER_CLEAR;

    float phaseTimer = 0.0f;
    float characterTimer = 0.0f;
    float talkSeTimer = 0.0f;

    // UTF-8文字列の先頭から何バイトまで表示するか。
    // 日本語1文字は複数バイトなので、単純な文字数ではなくバイト位置を保持する。
    std::size_t visibleByteCount = 0;

    bool active = false;
};

void Stage1ClearDialogInit(Stage1ClearDialog& dialog, const Texture2D& fallbackPortrait, const Font& font, AudioManager& audio);
void Stage1ClearDialogUnload(Stage1ClearDialog& dialog);
void Stage1ClearDialogBegin(Stage1ClearDialog& dialog);
void Stage1ClearDialogEnd(Stage1ClearDialog& dialog);
Stage1ClearDialogResult Stage1ClearDialogUpdate(Stage1ClearDialog& dialog, float dt);

// trueの間は、main.cpp側で停止中のステージ画面を先に描画する。
bool Stage1ClearDialogShouldDrawStageBehind(const Stage1ClearDialog& dialog);

// ステージ画面の上へ重ねる暗転と、会話専用画面の両方を描画する。
void Stage1ClearDialogDraw(const Stage1ClearDialog& dialog, int screenWidth, int screenHeight);