#pragma once
#include "raylib.h"
#include <string>

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
	bool touchedFromSide = false; // 追加
	float patrolMinX;
	float patrolMaxX;
	float patrolMaxY;
	float patrolMinY;
	float timer;
	std::string dialogKey = "";
	Texture2D texture{};
	Texture2D hitTexture{}; // 追加: 被弾時のテクスチャ
	float animTimer = 0.0f;
	int currentFrame = 0; // 現在のアニメーションフレーム
	bool facingRight = true; // 敵の向き
};

void EnemyInit(Enemy& enemy, EnemyType type, Vector2 position);
void EnemyCollision(Enemy& enemy, const Rectangle& player, float dt,Vector2& velocity);
void EnemyUpdate(Enemy& enemy, float dt, const Rectangle& player);
void EnemyDraw(const Enemy& enemy);
void EnemyReset(Enemy& enemy);
void EnemyLoadTextures();
void EnemyUnloadTextures();