#include "OjisanVisual.h"
#include <raylib.h>
#include <vector>

// OjisanVisual.cpp の役割: 右下固定UIとしてのおじさん表示と吹き出し描画を実装する。
OjisanVisual::OjisanVisual()
	: portrait{ 0,0,0,0,0 }, bubble{ 0,0,0,0,0 }, ojisanPunch{ 0,0,0,0,0 }, punchEffect{ 0,0,0,0,0 }, message(""),
	showMessage(false), timer(0.0f), duration(0.0f),
	scale(0.20f), margin(8.0f), fontSize(25),
	fontPtr(nullptr), overrideFontPtr(nullptr),
	useOverrideFont(false), useDefaultFontMode(false), showPunch(false)
{
}

void OjisanVisual::SetFont(const Font& f) {
    fontPtr = &f;
}

void OjisanVisual::Load() {
	portrait = LoadTexture("assets/images/ojisan/ojisan2.png");
	bubble = LoadTexture("assets/images/ojisan/bubble/bubble.png");
	ojisanPunch = LoadTexture("assets/images/ojisan/ojisan_punch.png");  // プレイ中のおじさん
	// ojisanPunch が読み込めない場合は portrait をフォールバック
	if (ojisanPunch.id == 0) {
		ojisanPunch = portrait;
	}
	punchEffect = LoadTexture("assets/images/ojisan/punch.png");  // パンチエフェクト
	// punchEffect が読み込めない場合は空のまま（後で id == 0 チェック）
}

void OjisanVisual::Unload() {
	if (portrait.id != 0) UnloadTexture(portrait);
	if (bubble.id != 0) UnloadTexture(bubble);
	// ojisanPunch が portrait と同じ場合は二重解放を避ける
	if (ojisanPunch.id != 0 && ojisanPunch.id != portrait.id) UnloadTexture(ojisanPunch);
	if (punchEffect.id != 0) UnloadTexture(punchEffect);
}

// 目的: 新しいメッセージ表示を開始し、表示時間とフォントモードを設定する。
void OjisanVisual::TriggerMessage(const std::string& text, float durationSeconds,
                                   const Font* overrideFont, bool forceDefaultFont) {
    message = text;
    duration = durationSeconds;
    timer = 0.0f;
    showMessage = true;
    useDefaultFontMode = forceDefaultFont;   // ← 追加

    if (!forceDefaultFont && overrideFont && overrideFont->texture.id != 0) {
        overrideFontPtr = overrideFont;
        useOverrideFont = true;
    }
    else {
        overrideFontPtr = nullptr;
        useOverrideFont = false;
    }
}
// 目的: メッセージ表示をクリアし、タイマーとフォントモードをリセットする。
void OjisanVisual::ClearMessage() {
	message.clear();
	showMessage = false;
	timer = 0.0f;
	duration = 0.0f;

	overrideFontPtr = nullptr;
	useOverrideFont = false;
	useDefaultFontMode = false;
}
void OjisanVisual::Update(float dt) {
	if (!showMessage) return;
	timer += dt;
	if (timer > duration) {
		showMessage = false;
	}
}

// スクリーン座標で常に右下に表示（UIとして振る舞う）
void OjisanVisual::Draw(int screenWidth, int screenHeight) {
	// 使用フォントを決定
	const Font* usedFontPtr = nullptr;
	Font defaultFontLocal = {};   // GetFontDefault() 用のローカルコピー

    if (useDefaultFontMode) {
        // コメントブロック等: 標準フォント（GetFontDefault）を使用
        defaultFontLocal = GetFontDefault();
        usedFontPtr = &defaultFontLocal;
    }
    else if (useOverrideFont && overrideFontPtr && overrideFontPtr->texture.id != 0) {
        usedFontPtr = overrideFontPtr;
    }
    else if (fontPtr && fontPtr->texture.id != 0) {
        usedFontPtr = fontPtr;
    }

    if (!usedFontPtr) return;
	// フォントが設定されていない場合は描画をスキップ
	if (usedFontPtr->texture.id == 0) return;

	// ポートレートは常に画面の右下（スクリーン座標）
	float pW = (float)portrait.width * scale;
	float pH = (float)portrait.height * scale;
	float px = (float)screenWidth - pW - margin;
	float py = (float)screenHeight - pH - margin;

	Rectangle srcP = { 0.0f, 0.0f, (float)portrait.width, (float)portrait.height };// 画面のどこを使うか
	Rectangle dstP = { px + pW * 0.5f, py + pH * 0.5f, pW, pH };//画面のどこにどの大きさで描画するか 
	Vector2 originP = { dstP.width * 0.5f, dstP.height * 0.5f };//描画の基準点（中心）

	/*ojisan画像を、画面全体から切り取って画面右下にscale倍のサイズで中心を基準に回転無しで元の色のまま描画する*/
	if (showPunch) {
		DrawTexturePro(ojisanPunch, srcP, dstP, originP, 0.0f, WHITE);// パンチ中のおじさん画像を描画
	}
	else {
		DrawTexturePro(portrait, srcP, dstP, originP, 0.0f, WHITE);// 通常のおじさん画像を描画
	}
	

	// メッセージが有効なら描画（ポートレートの左側に表示）
	if (showMessage && !message.empty()) {
		float padding = 50.0f;

		float maxInnerWidth = screenWidth * 0.33f - padding * 2.0f;
		if (maxInnerWidth < 80.0f) maxInnerWidth = 80.0f;

		Vector2 msgSize = MeasureTextEx(*usedFontPtr, message.c_str(), (float)fontSize, 1.0f);
		float innerW = msgSize.x;
		if (innerW > maxInnerWidth) innerW = maxInnerWidth;

		float bw = innerW + padding * 2.0f;
		float lineH = MeasureTextEx(*usedFontPtr, "あ", (float)fontSize, 1.0f).y;  // 日本語文字で高さ測定
		float bh = lineH * 4.0f + padding * 1.0f;

		float headX = px + pW * 0.35f;
		float headY = py + pH * 0.25f;

		float bx = headX - bw - 8.0f;
		float by = headY - bh * 1.05f;

		if (bx < margin) bx = margin;
		if (by < margin) by = margin;
		if (bx + bw > screenWidth - margin) bx = screenWidth - margin - bw;
		if (by + bh > screenHeight - margin) by = screenHeight - margin - bh;

		if (bubble.id != 0) {
			Rectangle srcB = { 0.0f, 0.0f, (float)bubble.width, (float)bubble.height };
			Rectangle dstB = { bx, by, bw, bh };
			DrawTexturePro(bubble, srcB, dstB, { 0.0f, 0.0f }, 0.0f, WHITE);
		}
		else {
			Rectangle r = { bx, by, bw, bh };
			DrawRectangleRounded(r, 0.15f, 8, Fade(RAYWHITE, 0.95f));
			DrawRectangleRoundedLines(r, 0.15f, 8, GRAY);
		}

		Rectangle textRec = { bx + padding, by + padding, innerW, bh - padding * 2.0f };

		DrawTextRecCompat(*usedFontPtr, message, textRec, (float)fontSize, 1.0f, true, BLACK);
	}
}

// DrawTextRecCompat は既存の実装を再利用
void OjisanVisual::DrawTextRecCompat(Font font, const std::string& text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint) {
	if (!wordWrap) {
		DrawTextEx(font, text.c_str(), { rec.x, rec.y }, fontSize, spacing, tint);
		return;
	}

	int cpCount = 0;
	int* cps = LoadCodepoints(text.c_str(), &cpCount);
	if (!cps || cpCount == 0) return;

	std::vector<std::vector<int>> linesCp;
	std::vector<int> curLine;
	for (int i = 0; i < cpCount; ++i) {
		curLine.push_back(cps[i]);
		char* utf8 = LoadUTF8(curLine.data(), (int)curLine.size());
		if (!utf8) continue;
		float w = MeasureTextEx(font, utf8, fontSize, spacing).x;
		UnloadUTF8(utf8);

		if (w > rec.width && curLine.size() > 1) {
			int last = curLine.back();
			curLine.pop_back();
			linesCp.push_back(curLine);
			curLine.clear();
			curLine.push_back(last);
		}
	}
	if (!curLine.empty()) linesCp.push_back(curLine);

	std::vector<std::string> lineStrs;
	std::vector<float> lineWidths;
	std::vector<float> lineHeights;
	lineStrs.reserve(linesCp.size());
	lineWidths.reserve(linesCp.size());
	lineHeights.reserve(linesCp.size());

	for (auto &lineCp : linesCp) {
		char* utf8 = LoadUTF8(lineCp.data(), (int)lineCp.size());
		if (!utf8) {
			lineStrs.emplace_back("");
			lineWidths.emplace_back(0.0f);
			lineHeights.emplace_back(0.0f);
			continue;
		}
		std::string s(utf8);
		float w = MeasureTextEx(font, utf8, fontSize, spacing).x;
		float h = MeasureTextEx(font, utf8, fontSize, spacing).y;
		lineStrs.push_back(std::move(s));
		lineWidths.push_back(w);
		lineHeights.push_back(h);
		UnloadUTF8(utf8);
	}

	float totalHeight = 0.0f;
	for (size_t i = 0; i < lineHeights.size(); ++i) totalHeight += lineHeights[i];
	if (lineHeights.size() >= 2) totalHeight += spacing * (float)(lineHeights.size() - 1);

	float startY = rec.y;
	if (totalHeight < rec.height) startY = rec.y + (rec.height - totalHeight) * 0.5f;

	float y = startY;
	for (size_t i = 0; i < lineStrs.size(); ++i) {
		const std::string &s = lineStrs[i];
		float w = lineWidths[i];
		float h = lineHeights[i];

		float x = rec.x;
		if (w < rec.width) x = rec.x + (rec.width - w) * 0.5f;
		DrawTextEx(font, s.c_str(), { x, y }, fontSize, spacing, tint);
		y += h + spacing;
	}

	UnloadCodepoints(cps);
}