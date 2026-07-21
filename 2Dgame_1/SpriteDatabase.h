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
	None = 0,//1
	Douka,//2
	MetalFloor01,//3
	MetalFloor02,//4
	MetalWall01,//5
	MetalWall02,//6
	Ladder01,//7
	PipeStraight,//8
	PipeCorner,//9
	Vent,//10
	Door,//11
	Fence,//12
	Beam,//13
	ground1,//14
	ground2,//15
	CursorPlatform,//16
	Dassh,//17
	Arrow,//18
	Buttom_break,//19
	Exitdoor_2,//20
	Exitdoor_1,//21
	Exitdoor2_fake,//22
	Drum_1,//23
	Drum_2,//24
	tempBlock,//25
	NextStage_1,//26
	NextStage_2,//27
	NextStage_fake,//28
	Bullet,//29
	Stage_2,//30
	Stage_3,//31
	Stage_4,//32
	Enemy_auto,//33
	DIE_block,//34
	Nedle3,//35
	IceBlock,//36
	Fake_clear,//37
	Jump_Space,//38
	Jump_N,//39

	//おじさん
	Ojisan_car,//40
	Ojisan_real1,//41
	Ojisan_real2,//42
	Ojisan_real3,//43
	Ojisan_kick,//44
	Ojisan_waruguti1,//45
	Ojisan_waruguti2,//46
	Ojisan_waruguti3,//47
	Ojisan_idle,//48
	Ojisan_notWalk,//49
	canWalk,//50
	notWalk,//51

	//パズル
	PuzzleBridge,//52
	PuzzleEdg,//53
	bored_brigename,//54
	bored_wataru,//55
	brige_road,//56
	brige_saku,//57
	brige_saku2,//58

	enemy_muteki,//59
	enemy_teki,//60
	mu,//61
	Ojisan_real5,//62
	give,//63
	punch_4,//64
	punch_comment,//65
	punch_comment_2,//66
	punch_comment_3,//67
	punch_comment_4,//68
	fuck,//69
	move,//70
	COUNT//71
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