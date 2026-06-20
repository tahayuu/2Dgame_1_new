#pragma once
#include "raylib.h"
#include <string>

enum class EnemyType
{
WALKER,
FLYER,
SHOOTER
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
	float patrolMinX;
	float patrolMaxX;
	float patrolMaxY;
	float patrolMinY;
	float timer;
	std::string dialogKey = ""; // ダイアログマネージャーからセリフを取得するためのキー
};

void EnemyInit(Enemy& enemy, EnemyType type, Vector2 position);
void EnemyCollision(Enemy& enemy, const Rectangle& player, float dt,Vector2& velocity);
void EnemyUpdate(Enemy& enemy, float dt, const Rectangle& player);
void EnemyDraw(const Enemy& enemy);
void EnemyReset(Enemy& enemy);