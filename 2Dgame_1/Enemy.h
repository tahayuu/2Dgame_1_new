#pragma once
#include "raylib.h"
#include <string>

// ================================================================
// Enemy.h の役割
// ---------------------------------------------------------------
// ・敵1体の状態(Enemy)と、敵更新に必要な公開APIを定義する。
// ・当たり判定状態(isHit/isStomped/touchedFromSide)は
//   物理判定と見た目切替の橋渡しとして使う。
// ================================================================

enum class EnemyType
{
	WALKER,
	FLYER,
	SHOOTER,
	JUMPCOPY,
};

struct Enemy{
	EnemyType type;
	Rectangle rect;
	Vector2 vel;
	Vector2 pos;
	float speed;
	float hp;
	bool isActive;
	bool PlayerTouch = false;
	bool isHit = false;   // 追加: プレイヤー接触中の見た目切替
	bool isStomped = false;   // 追加: 踏みつけ用
	bool touchedFromSide = false;
	float patrolMinX;
	float patrolMaxX;
	float patrolMaxY;
	float patrolMinY;
	float timer;
	std::string dialogKey = "";
	Texture2D texture{};
	Texture2D hitTexture{}; // 追加: 被弾時のテクスチャ
	Texture2D stompTexture{};  // 追加
	float animTimer = 0.0f;
	int currentFrame = 0; // 現在のアニメーションフレーム
	bool facingRight = true; // 敵の向き
	bool isDying = false;
	float deathTimer = 0.0f;  // 追加: 踏みつけ後の表示時間
};

// 目的: 敵1体の初期パラメータを種別ごとに設定する。
void EnemyInit(Enemy& enemy, EnemyType type, Vector2 position);
// 目的: プレイヤーとの接触を判定し、側面衝突/踏みつけを振り分ける。
void EnemyCollision(Enemy& enemy, const Rectangle& player, float dt,Vector2& velocity);
// 目的: 敵AIの移動と生存状態を更新する。
void EnemyUpdate(Enemy& enemy, float dt, const Rectangle& player);
// 目的: 敵の現在状態に応じた見た目を描画する。
void EnemyDraw(const Enemy& enemy);
void EnemyReset(Enemy& enemy);
// 目的: 敵テクスチャを一括読み込みする。
void EnemyLoadTextures();
// 目的: 敵テクスチャを一括解放する。
void EnemyUnloadTextures();
