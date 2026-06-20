#pragma once
#include "raylib.h"

struct StageTheme {
    Texture2D platformTop{};     // 最上行テクスチャ（草など）
    Texture2D platformBody{};    // 内部テクスチャ（土など）
    Texture2D platformLeft{};    // 左端列テクスチャ
    Texture2D platformRight{};   // 右端列テクスチャ
	Texture2D itemBlock{};// アイテムブロックのテクスチャ
	Texture2D nomalBlock{};//普通ブロックのテクスチャ
    float tileSize = 50.0f;
    bool hasTextures = false;
    bool useLeftRight = false;   // true: 左右モード / false: 上下モード
};

void StageThemeLoad(StageTheme& theme, const char* topPath, const char* bodyPath, float tileSize);
void StageThemeLoadLR(StageTheme& theme, const char* leftPath, const char* rightPath, float tileSize);
void StageThemeLoadAll(StageTheme& theme, const char* topPath, const char* bodyPath, const char* leftPath, const char* rightPath, float tileSize);
void StageThemeUnload(StageTheme& theme);
void DrawPlatformTextured(Rectangle rect, const StageTheme& theme);