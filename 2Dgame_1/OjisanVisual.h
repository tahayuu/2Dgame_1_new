#pragma once
#include "raylib.h"
#include <string>

class OjisanVisual {
public:
	Texture2D portrait;
	Texture2D bubble;
	Texture2D ojisanPunch;  // おじさんパンチ画像
	Texture2D punchEffect;  // パンチエフェクト画像
	std::string message;
	bool showMessage;
	float timer;
	float duration;
	float scale;
	float margin;
	int fontSize;
	// フォントはポインタで保持（外部でロードした Font を参照する）
	const Font* fontPtr;

	const Font* overrideFontPtr;
	bool useOverrideFont;
	bool useDefaultFontMode;   // ← 追加: true のとき GetFontDefault() を使用

	// パンチ表示用フラグ
	bool showPunch;

	OjisanVisual();

	void Load();
	void Unload();
	void Update(float dt);
	void Draw(int screenWidth, int screenHeight);
	// forceDefaultFont = true のとき GetFontDefault() で描画
	void TriggerMessage(const std::string& text, float durationSeconds = 3.0f,
						const Font* overrideFont = nullptr, bool forceDefaultFont = false);
	void DrawTextRecCompat(Font font, const std::string& text, Rectangle rec,
		float fontSize, float spacing, bool wordWrap, Color tint);
	// SetFont は参照からポインタを保持する（コピーしない）
	void SetFont(const Font& f);
};