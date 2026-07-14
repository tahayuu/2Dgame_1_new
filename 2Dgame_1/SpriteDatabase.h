#pragma once

#include "raylib.h"

// ================================================================
// SpriteId
// ----------------------------------------------------------------
// 「見た目（描画に使う画像パーツ）」だけを表すID。
// EditorObjectType（当たり判定・ギミックの種類）とは完全に別物として扱う。
//
// 例：EditorObjectType::PLATFORM（当たり判定は同じ「普通床」）でも、
//     SpriteId を変えるだけで MetalFloor01 / Ladder01 / Pipe... など
//     見た目だけを変えることができる。
//
// 新しい画像パーツを増やしたいときは、
//   1. ここに列挙子を追加する（COUNT の直前に追加すること！）
//   2. SpriteDatabase.cpp の g_spriteTable に対応する行を追加する
// の2箇所を変更すればOK。
// ================================================================
enum class SpriteId {
	None = 0,
	MetalFloor01,
	MetalFloor02,
	MetalWall01,
	MetalWall02,
	Ladder01,
	PipeStraight,
	PipeCorner,
	Vent,
	Door,
	Fence,
	Beam,
	ground1,
	ground2,
	CursorPlatform,
	Dassh,
	Arrow,
	Buttom_break,
	Exitdoor_2,
	Exitdoor_1,
	Exitdoor2_fake,
	Drum_1,
	Drum_2,
	tempBlock,
	NextStage_1,
	NextStage_2,
	NextStage_fake,
	Stage_2,
	Stage_3,
	Stage_4,
	Enemy_auto,
	DIE_block,
	Nedle3,
	IceBlock,
	Fake_clear,

	//おじさん
	Ojisan_car,
	Ojisan_real1,
	Ojisan_real2,
	Ojisan_real3,
	Ojisan_kick,
	Ojisan_waruguti1,
	Ojisan_waruguti2,
	Ojisan_waruguti3,
	COUNT
};

// ================================================================
// SpriteDatabase
// ----------------------------------------------------------------
// 「切り出し済みのPNG画像」を SpriteId ごとに1枚ずつ読み込んで管理する機能群。
// （アトラス画像から切り抜くのではなく、画像ファイル自体が1パーツ分になっている想定）
// ゲーム全体で1つしか存在しない想定なので、クラスではなく
// namespace + 関数群（シングルトン的な使い方）にしている。
//
// 画像ファイルは assets/images/sprites/ フォルダに置く想定。
// まだ画像を用意していない SpriteId はファイルが見つからないだけなので、
// 読み込みに失敗しても落ちない（DrawSprite が何もせず無視するだけ）。
// ================================================================
namespace SpriteDatabase {

	// ゲーム起動時に1回だけ呼ぶ。
	// SpriteId ごとの切り出し済みPNG画像をすべて読み込む。
	void Load();

	// ゲーム終了時に1回だけ呼ぶ。
	// 読み込んだテクスチャをすべて解放する。
	void Unload();

	// SpriteId に対応するテクスチャそのものを取得する。
	// 見つからない/未読み込みの場合は id==0 の空テクスチャを返す。
	Texture2D GetTexture(SpriteId id);

	// SpriteId の名前文字列を取得する（UI表示・保存用）。
	const char* GetSpriteName(SpriteId id);

	// 名前文字列から SpriteId を検索する（ファイル読み込み用）。
	// 見つからない場合は SpriteId::None を返す。
	SpriteId FindSpriteIdByName(const char* name);

	// dst の位置・大きさに、回転・反転・色味を適用してスプライトを描画する。
	// id が None の場合、またはテクスチャが読み込めていない場合は何も描画しない
	// （呼び出し側で「その場合は仮描画をする」という分岐をしてもらう想定）。
	//
	// rotation : 度数法（degree）。dst の中心を軸に回転する。
	// flipX/flipY : true にすると左右／上下を反転して描画する。
	void DrawSprite(SpriteId id, Rectangle dst, float rotation, bool flipX, bool flipY, Color tint);

}