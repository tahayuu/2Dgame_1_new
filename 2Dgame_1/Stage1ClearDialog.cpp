#include "Stage1ClearDialog.h"
#include <algorithm>
#include <cstring>
#include <string>

namespace {
    const char* GetDialogLine(int index)
    {
        static const char* lines[Stage1ClearDialog::LINE_COUNT] = {
            reinterpret_cast<const char*>(u8"ふーーむやはりこのていどじゃクリアされてしまうか"),
            reinterpret_cast<const char*>(u8"。。。。。。。"),
            reinterpret_cast<const char*>(u8"ふむ、やはりハードにすべきじゃな"),
            reinterpret_cast<const char*>(u8"イージーなんていわせないほどにな"),
            reinterpret_cast<const char*>(u8"ほれ「TAB」をおしてタイトルにもどるんじゃ"),
            reinterpret_cast<const char*>(u8"ぜんぶやりなおしじゃ")
        };

        if (index < 0 || index >= Stage1ClearDialog::LINE_COUNT) {
            return "";
        }

        return lines[index];
    }

    // セリフ番号と画像ファイルを対応させる。
    // 画像を変更したい場合は、この表のパスだけを変更すればよい。
    const char* GetPortraitPath(int index)
    {
        static const char* paths[Stage1ClearDialog::LINE_COUNT] = {
            "assets/images/ojisan/stage1_clear/dialog_01.png",
            "assets/images/ojisan/stage1_clear/dialog_02.png",
            "assets/images/ojisan/stage1_clear/dialog_03.png",
            "assets/images/ojisan/stage1_clear/dialog_04.png",
            "assets/images/ojisan/stage1_clear/dialog_05.png",
            "assets/images/ojisan/stage1_clear/dialog_06.png"
        };

        if (index < 0 || index >= Stage1ClearDialog::LINE_COUNT) {
            return nullptr;
        }

        return paths[index];
    }

    const Texture2D* GetCurrentPortrait(const Stage1ClearDialog& dialog)
    {
        if (dialog.lineIndex >= 0 && dialog.lineIndex < Stage1ClearDialog::LINE_COUNT) {
            const Texture2D& linePortrait = dialog.linePortraits[dialog.lineIndex];

            if (linePortrait.id != 0) {
                return &linePortrait;
            }
        }

        if (dialog.fallbackPortrait != nullptr && dialog.fallbackPortrait->id != 0) {
            return dialog.fallbackPortrait;
        }

        return nullptr;
    }

    float Clamp01(float value)
    {
        return std::clamp(value, 0.0f, 1.0f);
    }

    unsigned char ToAlphaByte(float normalizedAlpha)
    {
        return static_cast<unsigned char>(Clamp01(normalizedAlpha) * 255.0f);
    }

    // UTF-8の先頭バイトを調べ、この1文字が何バイトかを返す。
    std::size_t GetUtf8CharacterByteCount(unsigned char firstByte)
    {
        if ((firstByte & 0x80u) == 0x00u) return 1;
        if ((firstByte & 0xE0u) == 0xC0u) return 2;
        if ((firstByte & 0xF0u) == 0xE0u) return 3;
        if ((firstByte & 0xF8u) == 0xF0u) return 4;
        return 1;
    }

    // 現在のバイト位置から、UTF-8の次の1文字分だけ進める。
    std::size_t GetNextUtf8BytePosition(const char* text, std::size_t currentBytePosition)
    {
        if (text == nullptr) {
            return 0;
        }

        const std::size_t textByteLength = std::strlen(text);
        if (currentBytePosition >= textByteLength) {
            return textByteLength;
        }

        const unsigned char firstByte = static_cast<unsigned char>(text[currentBytePosition]);
        const std::size_t characterByteCount = GetUtf8CharacterByteCount(firstByte);

        return std::min(currentBytePosition + characterByteCount, textByteLength);
    }

    bool IsCurrentLineFullyVisible(const Stage1ClearDialog& dialog)
    {
        const char* line = GetDialogLine(dialog.lineIndex);
        return dialog.visibleByteCount >= std::strlen(line);
    }

    void StopTalkSound(Stage1ClearDialog& dialog)
    {
        if (dialog.audio != nullptr) {
            AudioStopSfx(*dialog.audio, SfxId::OjisanTalk);
        }
    }

    void PlayTalkSound(Stage1ClearDialog& dialog)
    {
        if (dialog.audio == nullptr || dialog.talkSeTimer > 0.0f) {
            return;
        }

        AudioPlaySfx(*dialog.audio, SfxId::OjisanTalk);
        dialog.talkSeTimer = Stage1ClearDialog::TALK_SE_INTERVAL_SECONDS;
    }

    void StartTypingCurrentLine(Stage1ClearDialog& dialog)
    {
        const char* line = GetDialogLine(dialog.lineIndex);

        dialog.phase = Stage1ClearDialogPhase::TYPING;
        dialog.phaseTimer = 0.0f;
        dialog.characterTimer = 0.0f;
        dialog.talkSeTimer = 0.0f;

        // 文字送り開始時に最初の1文字だけはすぐ表示する。
        dialog.visibleByteCount = GetNextUtf8BytePosition(line, 0);
        PlayTalkSound(dialog);

        if (IsCurrentLineFullyVisible(dialog)) {
            dialog.phase = Stage1ClearDialogPhase::READY;
        }
    }

    void ShowCurrentLineImmediately(Stage1ClearDialog& dialog)
    {
        dialog.visibleByteCount = std::strlen(GetDialogLine(dialog.lineIndex));
        dialog.characterTimer = 0.0f;
        dialog.talkSeTimer = 0.0f;
        dialog.phase = Stage1ClearDialogPhase::READY;
        StopTalkSound(dialog);
    }

    void PlayDialogEnterSound(Stage1ClearDialog& dialog)
    {
        if (dialog.audio != nullptr) {
            AudioPlaySfx(*dialog.audio, SfxId::DialogEnter);
        }
    }

    float GetFittedFontSize(const Font& font, const char* text, float preferredSize, float maxWidth)
    {
        const Vector2 measured = MeasureTextEx(font, text, preferredSize, 1.0f);
        if (measured.x <= maxWidth || measured.x <= 0.0f) {
            return preferredSize;
        }

        const float fitted = preferredSize * (maxWidth / measured.x);
        return std::max(fitted, 26.0f);
    }

    void DrawCenteredText(const Font& font, const char* text, float y, float fontSize, float screenWidth, Color color)
    {
        const Vector2 size = MeasureTextEx(font, text, fontSize, 1.0f);
        const float x = (screenWidth - size.x) * 0.5f;
        DrawTextEx(font, text, { x, y }, fontSize, 1.0f, color);
    }

    // 全文の幅を基準にX座標を固定する。
    // これにより1文字ずつ増えても文章全体が左右へ揺れない。
    void DrawTypewriterText(
        const Font& font,
        const char* fullText,
        std::size_t visibleByteCount,
        float y,
        float fontSize,
        float screenWidth,
        Color color
    ) {
        const std::string visibleText(fullText, visibleByteCount);
        const Vector2 fullTextSize = MeasureTextEx(font, fullText, fontSize, 1.0f);
        const float x = (screenWidth - fullTextSize.x) * 0.5f;

        DrawTextEx(font, visibleText.c_str(), { x, y }, fontSize, 1.0f, color);
    }

    void DrawPortraitInUpperHalf(
        const Texture2D& portrait,
        int screenWidth,
        int screenHeight,
        float alpha
    ) {
        if (portrait.id == 0 || portrait.width <= 0 || portrait.height <= 0) {
            return;
        }

        const float upperHeight = screenHeight * 0.48f;
        const float maxWidth = screenWidth * 0.58f;
        const float maxHeight = upperHeight - 20.0f;

        const float widthScale = maxWidth / static_cast<float>(portrait.width);
        const float heightScale = maxHeight / static_cast<float>(portrait.height);
        const float drawScale = std::min(widthScale, heightScale);

        const float drawWidth = portrait.width * drawScale;
        const float drawHeight = portrait.height * drawScale;
        const float drawX = (screenWidth - drawWidth) * 0.5f;
        const float drawY = upperHeight - drawHeight;

        const Color tint = { 255, 255, 255, ToAlphaByte(alpha) };

        DrawTexturePro(
            portrait,
            { 0.0f, 0.0f, static_cast<float>(portrait.width), static_cast<float>(portrait.height) },
            { drawX, drawY, drawWidth, drawHeight },
            { 0.0f, 0.0f },
            0.0f,
            tint
        );
    }
}

void Stage1ClearDialogInit(Stage1ClearDialog& dialog, const Texture2D& fallbackPortrait, const Font& font, AudioManager& audio)
{
    dialog.fallbackPortrait = &fallbackPortrait;
    dialog.font = &font;
    dialog.audio = &audio;
    dialog.lineIndex = 0;
    dialog.phase = Stage1ClearDialogPhase::WAIT_AFTER_CLEAR;
    dialog.phaseTimer = 0.0f;
    dialog.characterTimer = 0.0f;
    dialog.talkSeTimer = 0.0f;
    dialog.visibleByteCount = 0;
    dialog.active = false;

    for (int i = 0; i < Stage1ClearDialog::LINE_COUNT; i++) {
        dialog.linePortraits[i] = {};

        const char* path = GetPortraitPath(i);
        if (path != nullptr && FileExists(path)) {
            dialog.linePortraits[i] = LoadTexture(path);
        }
    }
}

void Stage1ClearDialogUnload(Stage1ClearDialog& dialog)
{
    StopTalkSound(dialog);

    for (int i = 0; i < Stage1ClearDialog::LINE_COUNT; i++) {
        if (dialog.linePortraits[i].id != 0) {
            UnloadTexture(dialog.linePortraits[i]);
            dialog.linePortraits[i] = {};
        }
    }

    dialog.fallbackPortrait = nullptr;
    dialog.font = nullptr;
    dialog.audio = nullptr;
    dialog.phaseTimer = 0.0f;
    dialog.characterTimer = 0.0f;
    dialog.talkSeTimer = 0.0f;
    dialog.visibleByteCount = 0;
    dialog.active = false;
}

void Stage1ClearDialogBegin(Stage1ClearDialog& dialog)
{
    StopTalkSound(dialog);
    dialog.lineIndex = 0;
    dialog.phase = Stage1ClearDialogPhase::WAIT_AFTER_CLEAR;
    dialog.phaseTimer = 0.0f;
    dialog.characterTimer = 0.0f;
    dialog.talkSeTimer = 0.0f;
    dialog.visibleByteCount = 0;
    dialog.active = true;
}

void Stage1ClearDialogEnd(Stage1ClearDialog& dialog)
{
    StopTalkSound(dialog);
    dialog.active = false;
    dialog.phaseTimer = 0.0f;
    dialog.characterTimer = 0.0f;
    dialog.talkSeTimer = 0.0f;
}

Stage1ClearDialogResult Stage1ClearDialogUpdate(Stage1ClearDialog& dialog, float dt)
{
    if (!dialog.active) {
        return Stage1ClearDialogResult::NONE;
    }

    if (dialog.talkSeTimer > 0.0f) {
        dialog.talkSeTimer -= dt;
        if (dialog.talkSeTimer < 0.0f) dialog.talkSeTimer = 0.0f;
    }

    switch (dialog.phase) {
    case Stage1ClearDialogPhase::WAIT_AFTER_CLEAR:
        dialog.phaseTimer += dt;

        if (dialog.phaseTimer >= Stage1ClearDialog::WAIT_AFTER_CLEAR_SECONDS) {
            dialog.phase = Stage1ClearDialogPhase::FADE_TO_BLACK;
            dialog.phaseTimer = 0.0f;
        }
        break;

    case Stage1ClearDialogPhase::FADE_TO_BLACK:
        dialog.phaseTimer += dt;

        if (dialog.phaseTimer >= Stage1ClearDialog::FADE_TO_BLACK_SECONDS) {
            dialog.phase = Stage1ClearDialogPhase::PORTRAIT_FADE_IN;
            dialog.phaseTimer = 0.0f;
        }
        break;

    case Stage1ClearDialogPhase::PORTRAIT_FADE_IN:
        dialog.phaseTimer += dt;

        if (dialog.phaseTimer >= Stage1ClearDialog::PORTRAIT_FADE_IN_SECONDS) {
            StartTypingCurrentLine(dialog);
        }
        break;

    case Stage1ClearDialogPhase::TYPING: {
        // 文字送り中のENTERは、現在の文章だけを一気に全文表示する。
        if (IsKeyPressed(KEY_ENTER)) {
            ShowCurrentLineImmediately(dialog);
            PlayDialogEnterSound(dialog);
            break;
        }

        dialog.characterTimer += dt;
        bool revealedCharacterThisFrame = false;

        while (dialog.characterTimer >= Stage1ClearDialog::CHARACTER_INTERVAL_SECONDS &&
            !IsCurrentLineFullyVisible(dialog)) {
            dialog.visibleByteCount = GetNextUtf8BytePosition(
                GetDialogLine(dialog.lineIndex),
                dialog.visibleByteCount
            );

            dialog.characterTimer -= Stage1ClearDialog::CHARACTER_INTERVAL_SECONDS;
            revealedCharacterThisFrame = true;
        }

        // 文字が増えたフレームだけ短い会話SEを鳴らす。
        // 処理落ちで複数文字進んでも、同じフレームでは1回だけ鳴らす。
        if (revealedCharacterThisFrame) {
            PlayTalkSound(dialog);
        }

        if (IsCurrentLineFullyVisible(dialog)) {
            dialog.phase = Stage1ClearDialogPhase::READY;
            StopTalkSound(dialog);
        }
        break;
    }

    case Stage1ClearDialogPhase::READY: {
        const bool isLastLine = (dialog.lineIndex >= Stage1ClearDialog::LINE_COUNT - 1);

        // 全文表示後のENTERは、次の文章へ進む。
        if (!isLastLine && IsKeyPressed(KEY_ENTER)) {
            dialog.lineIndex++;
            StartTypingCurrentLine(dialog);
            PlayDialogEnterSound(dialog);
        }

        // 最後のTABでは直接タイトルへ戻らず、main.cppへポーズ要求を返す。
        // dialog.activeは維持するため、ポーズから「ゲームにもどる」を選べば会話画面へ戻れる。
        if (isLastLine && IsKeyPressed(KEY_TAB)) {
            if (dialog.audio != nullptr) {
                AudioPlaySfx(*dialog.audio, SfxId::tabclick);
            }

            return Stage1ClearDialogResult::OPEN_PAUSE;
        }
        break;
    }
    }

    return Stage1ClearDialogResult::NONE;
}

bool Stage1ClearDialogShouldDrawStageBehind(const Stage1ClearDialog& dialog)
{
    if (!dialog.active) {
        return false;
    }

    return dialog.phase == Stage1ClearDialogPhase::WAIT_AFTER_CLEAR ||
        dialog.phase == Stage1ClearDialogPhase::FADE_TO_BLACK;
}

void Stage1ClearDialogDraw(const Stage1ClearDialog& dialog, int screenWidth, int screenHeight)
{
    if (!dialog.active || dialog.font == nullptr) {
        return;
    }

    // 待機中は停止したステージ画面をそのまま見せる。
    if (dialog.phase == Stage1ClearDialogPhase::WAIT_AFTER_CLEAR) {
        return;
    }

    // ステージ画面の上へ黒を徐々に重ねる。
    if (dialog.phase == Stage1ClearDialogPhase::FADE_TO_BLACK) {
        const float fadeProgress = dialog.phaseTimer / Stage1ClearDialog::FADE_TO_BLACK_SECONDS;
        DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, ToAlphaByte(fadeProgress) });
        return;
    }

    // ここから先は完全な黒背景の専用会話画面。
    ClearBackground(BLACK);

    const Texture2D* currentPortrait = GetCurrentPortrait(dialog);

    if (dialog.phase == Stage1ClearDialogPhase::PORTRAIT_FADE_IN) {
        const float portraitProgress = dialog.phaseTimer / Stage1ClearDialog::PORTRAIT_FADE_IN_SECONDS;

        if (currentPortrait != nullptr) {
            DrawPortraitInUpperHalf(*currentPortrait, screenWidth, screenHeight, portraitProgress);
        }

        return;
    }

    if (currentPortrait != nullptr) {
        DrawPortraitInUpperHalf(*currentPortrait, screenWidth, screenHeight, 1.0f);
    }

    const float upperHeight = screenHeight * 0.48f;
    const float textAreaY = upperHeight;
    const float textAreaHeight = screenHeight - textAreaY;

    DrawLineEx(
        { 80.0f, textAreaY },
        { screenWidth - 80.0f, textAreaY },
        2.0f,
        DARKGRAY
    );

    const char* fullLine = GetDialogLine(dialog.lineIndex);
    const float maxTextWidth = screenWidth - 160.0f;
    const float fontSize = GetFittedFontSize(*dialog.font, fullLine, 44.0f, maxTextWidth);
    const float lineHeight = MeasureTextEx(*dialog.font, fullLine, fontSize, 1.0f).y;
    const float textY = textAreaY + (textAreaHeight - lineHeight) * 0.38f;

    DrawTypewriterText(
        *dialog.font,
        fullLine,
        dialog.visibleByteCount,
        textY,
        fontSize,
        static_cast<float>(screenWidth),
        WHITE
    );

    const bool isLastLine = (dialog.lineIndex >= Stage1ClearDialog::LINE_COUNT - 1);
    const bool isTyping = (dialog.phase == Stage1ClearDialogPhase::TYPING);

    // 文字送り中はENTERで全文表示、全文表示後はENTERで次へ進む。
    // 最後の文章が全文表示された後だけTABを案内する。
    const char* guide = (isLastLine && !isTyping) ? "TAB : PAUSE" : "ENTER";
    const float guideSize = 30.0f;
    const Color guideColor = ((int)(GetTime() * 2.0) % 2 == 0) ? LIGHTGRAY : GRAY;

    DrawCenteredText(
        *dialog.font,
        guide,
        screenHeight - 54.0f,
        guideSize,
        static_cast<float>(screenWidth),
        guideColor
    );
}