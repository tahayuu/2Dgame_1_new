#pragma once
#include "raylib.h"

// ================================================================
// StageTheme.h の役割
// ---------------------------------------------------------------
// ・ステージ床のテクスチャセットをまとめて管理する。
// ・見た目管理専用であり、当たり判定ロジックは持たない。
// ================================================================

struct StageTheme {
    Texture2D platformTop{};     // 最上行テクスチャ（草など）
    Texture2D platformBody{};    // 内部テクスチャ（土など）
    Texture2D platformLeft{};    // 左端列テクスチャ
    Texture2D platformRight{};   // 右端列テクスチャ
	Texture2D itemBlock{};// アイテムブロックのテクスチャ
	Texture2D nomalBlock{};//普通ブロックのテクスチャ
	Texture2D arrowTex{};	  // 矢印のテクスチャ
    float tileSize = 50.0f;
    bool hasTextures = false;
    bool useLeftRight = false;   // true: 左右モード / false: 上下モード
};
// 目的: アイテムブロック/通常ブロック/矢印など、ギミック用の画像を読み込む。
void StageThemeLoadObjectTextures(
    StageTheme& theme,
    const char* itemBlockPath,
    const char* normalBlockPath,
    const char* arrowPath
);

void StageThemeLoadObjectTextures(StageTheme& theme);
// 目的: 上面+内部の2テクスチャ構成で床テーマを読み込む。
void StageThemeLoad(StageTheme& theme, const char* topPath, const char* bodyPath, float tileSize);
// 目的: 左右端テクスチャを使う横方向テーマを読み込む。
void StageThemeLoadLR(StageTheme& theme, const char* leftPath, const char* rightPath, float tileSize);
// 目的: 上面/内部/左右端をまとめて読み込み、フルテーマを構築する。
void StageThemeLoadAll(StageTheme& theme, const char* topPath, const char* bodyPath, const char* leftPath, const char* rightPath, float tileSize);
// 目的: 読み込み済みテクスチャを解放し、themeを初期値へ戻す。
void StageThemeUnload(StageTheme& theme);
// 目的: 1つの床矩形をテーマ設定に従って分割描画する。
void DrawPlatformTextured(Rectangle rect, const StageTheme& theme);
