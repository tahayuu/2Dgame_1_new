#include "StageClearDialog.h"
#include "DialogManager.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace {
    const char* GetCurrentLine(const StageClearDialog& dialog)
    {
        if (dialog.lineIndex < 0 || dialog.lineIndex >= dialog.lineCount) {
            return "";
        }

        return dialog.lines[dialog.lineIndex].c_str();
    }

    std::string MakeDialogKey(int stageNumber, int lineNumber)
    {
        return "stage" + std::to_string(stageNumber) +
            "_clear_" + std::to_string(lineNumber);
    }

    void ClearDialogLines(StageClearDialog& dialog)
    {
        for (std::string& line : dialog.lines) {
            line.clear();
        }

        dialog.lineCount = 0;
    }

    void LoadDialogLines(StageClearDialog& dialog, int stageNumber)
    {
        ClearDialogLines(dialog);

        // ojisan_lines.text の stageN_clear_1 ～ stageN_clear_10 を順番に読む。
        // 途中の番号が抜けた時点で、そのステージの会話終了とする。
        for (int i = 0; i < StageClearDialog::MAX_LINE_COUNT; i++) {
            const std::string key = MakeDialogKey(stageNumber, i + 1);
            const std::string line = DialogManager::Instance().Get(key, "");

            if (line.empty()) {
                break;
            }

            dialog.lines[dialog.lineCount] = line;
            dialog.lineCount++;
        }

        // まだ会話文を用意していない新ステージでも演出が止まらないようにする。
        if (dialog.lineCount == 0) {
            dialog.lines[0] = "ステージ" + std::to_string(stageNumber) + "をクリアしたか";
            dialog.lines[1] = "なかなかやるではないか";

            if (stageNumber == 1) {
                dialog.lines[2] = "ほれ「TAB」をおしてタイトルにもどるんじゃ";
            }
            else {
                dialog.lines[2] = "ほれ、つぎのステージをえらぶんじゃ";
            }

            dialog.lineCount = 3;
        }
    }

    void UnloadLinePortraits(StageClearDialog& dialog)
    {
        for (Texture2D& portrait : dialog.linePortraits) {
            if (portrait.id != 0) {
                UnloadTexture(portrait);
                portrait = {};
            }
        }
    }

    std::string MakePortraitPath(int stageNumber, int lineNumber)
    {
        char path[256]{};
        std::snprintf(
            path,
            sizeof(path),
            "assets/images/ojisan/stage%d_clear/dialog_%02d.png",
            stageNumber,
            lineNumber
        );
        return path;
    }

    void LoadLinePortraits(StageClearDialog& dialog)
    {
        UnloadLinePortraits(dialog);

        for (int i = 0; i < dialog.lineCount; i++) {
            const std::string path = MakePortraitPath(dialog.clearedStage, i + 1);

            if (FileExists(path.c_str())) {
                dialog.linePortraits[i] = LoadTexture(path.c_str());
            }
        }
    }

    const Texture2D* GetCurrentPortrait(const StageClearDialog& dialog)
    {
        if (dialog.lineIndex >= 0 && dialog.lineIndex < dialog.lineCount) {
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

    std::size_t GetUtf8CharacterByteCount(unsigned char firstByte)
    {
        if ((firstByte & 0x80u) == 0x00u) return 1;
        if ((firstByte & 0xE0u) == 0xC0u) return 2;
        if ((firstByte & 0xF0u) == 0xE0u) return 3;
        if ((firstByte & 0xF8u) == 0xF0u) return 4;
        return 1;
    }

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

    bool IsCurrentLineFullyVisible(const StageClearDialog& dialog)
    {
        return dialog.visibleByteCount >= std::strlen(GetCurrentLine(dialog));
    }

    void StopTalkSound(StageClearDialog& dialog)
    {
        if (dialog.audio != nullptr) {
            AudioStopSfx(*dialog.audio, SfxId::OjisanTalk);
        }
    }

    void PlayTalkSound(StageClearDialog& dialog)
    {
        if (dialog.audio == nullptr || dialog.talkSeTimer > 0.0f) {
            return;
        }

        AudioPlaySfx(*dialog.audio, SfxId::OjisanTalk);
        dialog.talkSeTimer = StageClearDialog::TALK_SE_INTERVAL_SECONDS;
    }

    void StartTypingCurrentLine(StageClearDialog& dialog)
    {
        const char* line = GetCurrentLine(dialog);

        dialog.phase = StageClearDialogPhase::TYPING;
        dialog.phaseTimer = 0.0f;
        dialog.characterTimer = 0.0f;
        dialog.talkSeTimer = 0.0f;
        dialog.visibleByteCount = GetNextUtf8BytePosition(line, 0);
        PlayTalkSound(dialog);

        if (IsCurrentLineFullyVisible(dialog)) {
            dialog.phase = StageClearDialogPhase::READY;
        }
    }

    void ShowCurrentLineImmediately(StageClearDialog& dialog)
    {
        dialog.visibleByteCount = std::strlen(GetCurrentLine(dialog));
        dialog.phaseTimer = 0.0f;
        dialog.characterTimer = 0.0f;
        dialog.talkSeTimer = 0.0f;
        dialog.phase = StageClearDialogPhase::READY;
        StopTalkSound(dialog);
    }

    void PlayDialogEnterSound(StageClearDialog& dialog)
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

    void DrawPortraitInUpperHalf(const Texture2D& portrait, int screenWidth, int screenHeight, float alpha)
    {
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

        DrawTexturePro(
            portrait,
            { 0.0f, 0.0f, static_cast<float>(portrait.width), static_cast<float>(portrait.height) },
            { drawX, drawY, drawWidth, drawHeight },
            { 0.0f, 0.0f },
            0.0f,
            { 255, 255, 255, ToAlphaByte(alpha) }
        );
    }
}

void StageClearDialogInit(StageClearDialog& dialog, const Texture2D& fallbackPortrait, const Font& font, AudioManager& audio)
{
    dialog.fallbackPortrait = &fallbackPortrait;
    dialog.font = &font;
    dialog.audio = &audio;
    dialog.active = false;
}

void StageClearDialogUnload(StageClearDialog& dialog)
{
    StopTalkSound(dialog);
    UnloadLinePortraits(dialog);
    ClearDialogLines(dialog);

    dialog.fallbackPortrait = nullptr;
    dialog.font = nullptr;
    dialog.audio = nullptr;
    dialog.clearedStage = 0;
    dialog.active = false;
}

void StageClearDialogBegin(StageClearDialog& dialog, int clearedStage)
{
    StopTalkSound(dialog);

    dialog.clearedStage = clearedStage;
    LoadDialogLines(dialog, clearedStage);
    LoadLinePortraits(dialog);

    dialog.lineIndex = 0;
    dialog.phase = StageClearDialogPhase::WAIT_AFTER_CLEAR;
    dialog.phaseTimer = 0.0f;
    dialog.characterTimer = 0.0f;
    dialog.talkSeTimer = 0.0f;
    dialog.visibleByteCount = 0;
    dialog.active = true;
}

void StageClearDialogEnd(StageClearDialog& dialog)
{
    StopTalkSound(dialog);
    UnloadLinePortraits(dialog);
    ClearDialogLines(dialog);

    dialog.clearedStage = 0;
    dialog.lineIndex = 0;
    dialog.phaseTimer = 0.0f;
    dialog.characterTimer = 0.0f;
    dialog.talkSeTimer = 0.0f;
    dialog.visibleByteCount = 0;
    dialog.active = false;
}

StageClearDialogResult StageClearDialogUpdate(StageClearDialog& dialog, float dt)
{
    if (!dialog.active || dialog.lineCount <= 0) {
        return StageClearDialogResult::NONE;
    }

    if (dialog.talkSeTimer > 0.0f) {
        dialog.talkSeTimer -= dt;
        if (dialog.talkSeTimer < 0.0f) dialog.talkSeTimer = 0.0f;
    }

    switch (dialog.phase) {
    case StageClearDialogPhase::WAIT_AFTER_CLEAR:
        dialog.phaseTimer += dt;

        if (dialog.phaseTimer >= StageClearDialog::WAIT_AFTER_CLEAR_SECONDS) {
            dialog.phase = StageClearDialogPhase::FADE_TO_BLACK;
            dialog.phaseTimer = 0.0f;
        }
        break;

    case StageClearDialogPhase::FADE_TO_BLACK:
        dialog.phaseTimer += dt;

        if (dialog.phaseTimer >= StageClearDialog::FADE_TO_BLACK_SECONDS) {
            dialog.phase = StageClearDialogPhase::PORTRAIT_FADE_IN;
            dialog.phaseTimer = 0.0f;
        }
        break;

    case StageClearDialogPhase::PORTRAIT_FADE_IN:
        dialog.phaseTimer += dt;

        if (dialog.phaseTimer >= StageClearDialog::PORTRAIT_FADE_IN_SECONDS) {
            StartTypingCurrentLine(dialog);
        }
        break;

    case StageClearDialogPhase::TYPING: {
        if (IsKeyPressed(KEY_ENTER)) {
            ShowCurrentLineImmediately(dialog);
            PlayDialogEnterSound(dialog);
            break;
        }

        dialog.characterTimer += dt;
        bool revealedCharacterThisFrame = false;

        while (dialog.characterTimer >= StageClearDialog::CHARACTER_INTERVAL_SECONDS &&
            !IsCurrentLineFullyVisible(dialog)) {
            dialog.visibleByteCount = GetNextUtf8BytePosition(
                GetCurrentLine(dialog),
                dialog.visibleByteCount
            );

            dialog.characterTimer -= StageClearDialog::CHARACTER_INTERVAL_SECONDS;
            revealedCharacterThisFrame = true;
        }

        if (revealedCharacterThisFrame) {
            PlayTalkSound(dialog);
        }

        if (IsCurrentLineFullyVisible(dialog)) {
            dialog.phaseTimer = 0.0f;
            dialog.phase = StageClearDialogPhase::READY;
            StopTalkSound(dialog);
        }
        break;
    }

    case StageClearDialogPhase::READY: {
        const bool isLastLine = (dialog.lineIndex >= dialog.lineCount - 1);

        if (!isLastLine && IsKeyPressed(KEY_ENTER)) {
            dialog.lineIndex++;
            StartTypingCurrentLine(dialog);
            PlayDialogEnterSound(dialog);
            break;
        }

        // ステージ2以降は、最後のセリフが全文表示されたあとに
        // ENTERを押したときだけステージ選択へ進む。
        // 文字送り中のENTERは全文表示に使われるため、
        // ステージ選択へ進むには全文表示後にもう一度ENTERを押す。
        if (isLastLine && dialog.clearedStage >= 2 && IsKeyPressed(KEY_ENTER)) {
            PlayDialogEnterSound(dialog);
            return StageClearDialogResult::GO_TO_STAGE_SELECT;
        }

        // ステージ1だけは従来どおりTABでポーズ画面を開き、
        // タイトル画面2へ戻る流れを使用する。
        if (isLastLine && dialog.clearedStage == 1 && IsKeyPressed(KEY_TAB)) {
            if (dialog.audio != nullptr) {
                AudioPlaySfx(*dialog.audio, SfxId::tabclick);
            }

            return StageClearDialogResult::OPEN_PAUSE;
        }
        break;
    }
    }

    return StageClearDialogResult::NONE;
}

bool StageClearDialogShouldDrawStageBehind(const StageClearDialog& dialog)
{
    if (!dialog.active) {
        return false;
    }

    return dialog.phase == StageClearDialogPhase::WAIT_AFTER_CLEAR ||
        dialog.phase == StageClearDialogPhase::FADE_TO_BLACK;
}

void StageClearDialogDraw(const StageClearDialog& dialog, int screenWidth, int screenHeight)
{
    if (!dialog.active || dialog.font == nullptr || dialog.lineCount <= 0) {
        return;
    }

    if (dialog.phase == StageClearDialogPhase::WAIT_AFTER_CLEAR) {
        return;
    }

    if (dialog.phase == StageClearDialogPhase::FADE_TO_BLACK) {
        const float fadeProgress = dialog.phaseTimer / StageClearDialog::FADE_TO_BLACK_SECONDS;
        DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, ToAlphaByte(fadeProgress) });
        return;
    }

    ClearBackground(BLACK);

    const Texture2D* currentPortrait = GetCurrentPortrait(dialog);

    if (dialog.phase == StageClearDialogPhase::PORTRAIT_FADE_IN) {
        const float portraitProgress = dialog.phaseTimer / StageClearDialog::PORTRAIT_FADE_IN_SECONDS;

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

    const char* fullLine = GetCurrentLine(dialog);
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

    const bool isLastLine = (dialog.lineIndex >= dialog.lineCount - 1);
    const bool isTyping = (dialog.phase == StageClearDialogPhase::TYPING);

    const char* guide = "ENTER";

    if (isLastLine && !isTyping) {
        guide = (dialog.clearedStage >= 2)
            ? "ENTER : STAGE SELECT"
            : "TAB : PAUSE";
    }

    const Color guideColor = ((int)(GetTime() * 2.0) % 2 == 0) ? LIGHTGRAY : GRAY;

    DrawCenteredText(
        *dialog.font,
        guide,
        screenHeight - 54.0f,
        30.0f,
        static_cast<float>(screenWidth),
        guideColor
    );
}
