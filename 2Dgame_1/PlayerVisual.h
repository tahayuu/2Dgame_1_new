#pragma once
#include "raylib.h"

// ================================================================
// PlayerVisual.h の役割
// ---------------------------------------------------------------
// ・プレイヤー見た目（アニメーション状態）と背景見た目データを定義する。
// ・ゲームロジック(PlayerState)とは分離し、描画責務のみ持つ。
// ================================================================
//パーティクル1粒
static constexpr int MAX_PLAYER_PARTICLES = 32;

struct PlayerParticle {
	Vector2 pos;
	Vector2 vel;
	float life;      //残り寿命（秒）
	float maxLife;    //初期寿命
	float size;       //サイズ
	Color color;
	bool active;
};

//パーティクルエフェクト管理
struct PlayerParticleEffect {
	PlayerParticle particles[MAX_PLAYER_PARTICLES]{};
	float spawnTimer = 0.0f;
	float spawnInterval = 0.06f; //生成間隔（秒）
};



struct  PlayerVisual {
	Texture2D texIdle{};
	Texture2D texRun{};
	Texture2D texJump{};
	Texture2D texFall{};
	Texture2D texidleturn{};
	Texture2D texDeath{};

	int idleFrame = 0;//いま何コマ目の画像を使っているか
	int runFrame = 0;
	int jumpFrame = 0;
	int fallFrame = 0;
	int deathFrame = 0;

	float idleTimer = 0.0f;//アニメーション用タイマー
	float runTimer = 0.0f;
	float jumpTimer = 0.0f;
	float fallTimer = 0.0f;
	float deathTimer = 0.0f;

	const float idleFPS = 10.0f;//待機アニメの1秒あたりのコマ数
	const float runFPS = 12.0f;
	const float jumpFPS = 8.0f;
	const float fallFPS = 8.0f;
	const float deathFPS = 10.0f;

	bool facingRight = true;//向き
	bool isRunning = false;//走っているか
	bool isJumping = false;//ジャンプ中か
	bool isFalling = false;//落下中か
	bool isDying = false;//死亡中か

	float drawSvale = 5.0f;//見た目の拡大率
	int frameCount = 10;//アニメのコマ数
	int jumpFrameCount  = 6;
	int fallFrameCount  = 4;
	int deathFrameCount = 23;

	PlayerParticleEffect jumpEffect{};
};
 
struct StageVisual {
	Texture2D background{};
	Texture2D selectBackground{};
	Texture2D stage2{};
	Texture2D texFloor{};
	Texture2D texSpike{};
	Texture2D texdeath{};
	Texture2D title{};
};



// プレイヤーの見た目用データ読み込み・解放
void PlayerVisualLoad(PlayerVisual& pv);
void PlayerVisualUnload(PlayerVisual& pv);
// ステージの見た目用データ読み込み
void StageVisualLoad(StageVisual& sv);
void StageVisualUnload(StageVisual& sv);
//アニメ更新（速度や地面状態から/待機を決めてフレーム更新）
void PlayerVisualUpdate(PlayerVisual& pv, float dt,const Vector2& velocity,bool onGround);

//プレイヤー描画（当たり判定plyaerはそののまま返す）
void PlayerVisualDraw(const PlayerVisual& pv, const Rectangle& player, const Vector2& velocity, bool gravityReversed, float layerScale = 1.0f); 

//パーティクルエフェクト更新・描画
void PlayerParticleUpdate(PlayerParticleEffect& effect, float dt, const Rectangle& player, bool hasJumpBoost);
void PlayerParticleDraw(const PlayerParticleEffect& effect);

//ステージ背景描画（画面全体に描画）
void StageVisualDrawScreen(const StageVisual& sv, int screenW, int screenH);
//ステージ背景描画（画面全体に描画）
void StageVisualDrawWorld(const StageVisual& sv, float worldX, float worldY, float worldW, float worldH);
//タイトル画面描画
void TitleVisualDrawScreen(const StageVisual& sv, int screenW, int screenH);