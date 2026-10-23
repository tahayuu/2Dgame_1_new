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
	Texture2D normalBlock{};//普通ブロックのテクスチャ
	Texture2D touchBlock{};//接触ブロックのテクスチャ
	Texture2D arrowTex{};	  // 矢印のテクスチャ
	Texture2D bulletTex{};	  // 砲台のテクスチャ
	Texture2D actionButtonOff{}; //アクションボタンのテクスチャ（OFF）
	Texture2D actionButtonOn{};  //アクションボタンのテクスチャ（ON）
	Texture2D magnetTex{};	  // 磁石のテクスチャ
	Texture2D magnetEffectTex{}; // 磁石のエフェクトテクスチャ
	Texture2D tempBlockTex{};    // 一時的に表れるブロック（透明ブロック）のテクスチャ
	Texture2D circleTex{};       // 円軌道床のテクスチャ

	float tileSize = 50.0f;
    bool hasTextures = false;
    bool useLeftRight = false;   // true: 左右モード / false: 上下モード
};
// 目的: アイテムブロック/通常ブロック/矢印など、ギミック用の画像を読み込む。
void StageThemeLoadObjectTextures(
    StageTheme& theme,
    const char* itemBlockPath,
    const char* normalBlockPath,
    const char* arrowPath,
    const char* bulletPath,
	const char* ActionButtn_offPath,
	const char* ActionButtn_onPath,
	const char* touchBlockPath,
	const char* magnetPath,
	const char* magnetEffectPath,
	const char* tempBlockPath,
	const char* circlePath
);

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
