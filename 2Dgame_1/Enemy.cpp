#include"Enemy.h"
#include"raylib.h"
#include"StageCollision.h"
#include<cmath>

// テクスチャの静的管理
static Texture2D walkerTexture{};
static Texture2D flyerTexture{};
static Texture2D jumpcopyTexture{};
static Texture2D hitTexture{};
static Texture2D stompTexture{};
static bool texturesLoaded = false;

// スプライト設定（既存定数の近く）
// 画像全体の1コマサイズ
const int SPRITE_FRAME_WIDTH = 240;
const int SPRITE_FRAME_HEIGHT = 821;
const int SPRITE_FRAMES_PER_ROW = 8;
const float ANIMATION_SPEED = 0.1f;

// 当たり判定
const float ENEMY_HITBOX_W = 56.0f;
const float ENEMY_HITBOX_H = 56.0f;

// 見た目
const float ENEMY_DRAW_SCALE = 2.5f;
const float ENEMY_DRAW_Y_OFFSET = -13.0f; // 少し上に寄せる

// 切り取り余白
const float ENEMY_CROP_LEFT = 10.0f;      // 左を少し切る
const float ENEMY_CROP_RIGHT = 10.0f;     // 右を少し切る
const float ENEMY_CROP_TOP = 0.0f;
const float ENEMY_CROP_BOTTOM = 0.0f;

void EnemyLoadTextures() {
	if (texturesLoaded) return;

	walkerTexture = LoadTexture("assets/images/enemy/enemy1_walk.png");
	flyerTexture = LoadTexture("assets/images/enemy/enemy1_Fly.png");
	jumpcopyTexture = LoadTexture("assets/images/enemy/enemy1_walk.png");
	hitTexture = LoadTexture("assets/images/enemy/enemy1_Walk_kill_2.png"); 
	stompTexture = LoadTexture("assets/images/enemy/enemy1_stomp.png");

	TraceLog(LOG_INFO, "Enemy Textures Loaded - Walker ID: %d, Flyer ID: %d, JumpCopy ID: %d",
		walkerTexture.id, flyerTexture.id, jumpcopyTexture.id);

	texturesLoaded = true;
}

void EnemyUnloadTextures() {
	if (!texturesLoaded) return;
	UnloadTexture(walkerTexture);
	UnloadTexture(flyerTexture);
	UnloadTexture(jumpcopyTexture);
	UnloadTexture(hitTexture);
	UnloadTexture(stompTexture); // 追加
	texturesLoaded = false;
}

// Enemy 型を使ってテクスチャ ID を取得する関数
static Texture2D GetEnemyTexture(EnemyType type) {
	switch (type) {
	case EnemyType::WALKER:
		return walkerTexture;
	case EnemyType::FLYER:
		return flyerTexture;
	case EnemyType::JUMPCOPY:
		return jumpcopyTexture;
	default:
		return Texture2D{};
	}
}

//==========================
// static
//==========================
static bool CheckOverlapXEnemy(const Rectangle& player, const Enemy& enemy) {
	float playerLeft = player.x;
	float playerRight = player.x + player.width;
	float objectLeft = enemy.rect.x;
	float objectRight = enemy.rect.x + enemy.rect.width;
	bool overlapX = (playerRight > objectLeft) && (playerLeft < objectRight);
	return overlapX;
}

static bool CheckOverlapYEnemy(const Rectangle& player, const Enemy& enemy) {
	float playerTop = player.y;
	float playerBottom = player.y + player.height;
	float hazardTop = enemy.rect.y;
	float hazardBottom = enemy.rect.y + enemy.rect.height;
	bool overlapY = (playerBottom > hazardTop ) && (playerTop < hazardBottom);
	return overlapY;
}

static bool CheckNealapYEnemy(const Rectangle& player, const Enemy& enemy, float tolerance) {
	float playerTop = player.y;
	float playerBottom = player.y + player.height;
	float hazardTop = enemy.rect.y;
	float hazardBottom = enemy.rect.y + enemy.rect.height;
	bool overlapY = (playerBottom > hazardTop - tolerance) && (playerTop < hazardBottom);
	return overlapY;
}

static void EnemyCollisionErase(Enemy& enemy, const Rectangle& player, float dt, Vector2& velocity, float Y, float X) {
	if (!CheckOverlapXEnemy(player, enemy)) return;
	bool overlapY = CheckOverlapYEnemy(player, enemy);

	float playerCenterX = player.x + player.width * 0.5f;
	float playerCenterY = player.y + player.height * 0.5f;
	float enemyCenterX = enemy.rect.x + enemy.rect.width * 0.5f;
	float enemyCenterY = enemy.rect.y + enemy.rect.height * 0.5f;
	float dx = playerCenterX - enemyCenterX;
	float dy = playerCenterY - enemyCenterY;
	bool isMostlyHorizontal = (fabsf(dx) > fabsf(dy));

	const float stompTolerance = 15.0f;
	const float stompVelocityThreshold = 50.0f;

	float enemyTop = enemy.rect.y;
	float enemyBottom = enemy.rect.y + enemy.rect.height;
	float playerBottom = player.y + player.height;
	float playerTop = player.y;

	bool isCloseAbove = (playerBottom <= enemyTop + stompTolerance);
	bool isFallingEnough = (velocity.y > stompVelocityThreshold);

	bool isCloseBelow = (playerTop >= enemyBottom - stompTolerance);
	bool isRisingEnough = (velocity.y < -stompVelocityThreshold);

	if (!isMostlyHorizontal) {
		if (overlapY && isCloseAbove && isFallingEnough) {
			enemy.hp -= 1;
			enemy.isStomped = true;
			enemy.deathTimer = 0.0f;
			if (velocity.x < 0.0f) velocity.x = -X;
			else if (velocity.x > 0.0f) velocity.x = X;
			velocity.y = -Y;
		}
		else if (overlapY && isCloseBelow && isRisingEnough) {
			enemy.PlayerTouch = true;
		}
	}
	else {
		enemy.PlayerTouch = true;
		enemy.touchedFromSide = true;   // プレイヤーが側面から当たった
		enemy.isHit = true;             // 追加: 側面衝突時だけテクスチャ切り替え
	}
}

static bool playerSence(const Rectangle& player,const Enemy& enemy,float rangeX) {
	const float playerCenterX = player.x + player.width * 0.5f;
	const float enemyCenterX = enemy.rect.x + enemy.rect.width * 0.5f;
	return fabsf(playerCenterX - enemyCenterX) <= rangeX;
}

//==========================
// 公開関数の定義
//==========================
void EnemyInit(Enemy& enemy, EnemyType type, Vector2 spawnPos) {
	enemy.type = type;
	enemy.pos = spawnPos;
	enemy.vel = { 0.0f,0.0f };
	enemy.isActive = true;
	enemy.speed = 100.0f;
	enemy.hp = 1;
	enemy.PlayerTouch = false;
	enemy.isHit = false;
	enemy.isStomped = false;
	enemy.deathTimer = 0.0f;

	enemy.rect = { spawnPos.x, spawnPos.y, ENEMY_HITBOX_W, ENEMY_HITBOX_H };
	enemy.texture = GetEnemyTexture(type);
	enemy.hitTexture = hitTexture;
	enemy.stompTexture = stompTexture; // 追加

	switch (type)
	{
	case EnemyType::WALKER:
		enemy.vel = { 100, 100 };
		enemy.hp = 1;
		enemy.patrolMinX = spawnPos.x - 100.0f;
		enemy.patrolMaxX = spawnPos.x + 100.0f;
		break;

	case EnemyType::FLYER:
		enemy.vel = { 100.0f, 100.0f };
		enemy.hp = 1;
		enemy.patrolMinY = spawnPos.y - 100.0f;
		enemy.patrolMaxY = spawnPos.y + 100.0f;
		break;

	case EnemyType::JUMPCOPY:
		enemy.vel = { 100.0f, 100.0f };
		enemy.hp = 1;
		enemy.patrolMinX = spawnPos.x - 100.0f;
		enemy.patrolMaxX = spawnPos.x + 100.0f;
		enemy.patrolMinY = spawnPos.y;
		break;
	}

	enemy.rect.x = enemy.pos.x;
	enemy.rect.y = enemy.pos.y;
}

void EnemyCollision(Enemy& enemy, const Rectangle& player, float dt, Vector2& velocity) {
	if (!enemy.isActive) return;
	enemy.timer += dt;
	bool overlapX = CheckOverlapXEnemy(player, enemy);
	switch (enemy.type)
	{
	case EnemyType::WALKER: {
		EnemyCollisionErase(enemy, player, dt, velocity, 200, 100);
		break;
	}
	case EnemyType::FLYER: {
		EnemyCollisionErase(enemy, player, dt, velocity, 200, 100);
		break;
	}
	case EnemyType::JUMPCOPY: {
		EnemyCollisionErase(enemy, player, dt, velocity, 300, 150);
		break;
	}
	}
}

void EnemyUpdate(Enemy& enemy, float dt, const Rectangle& player){
	if (!enemy.isActive) return;

	if (enemy.isStomped) {
		enemy.deathTimer += dt;
		if (enemy.deathTimer >= 0.15f) {
			enemy.isActive = false;
		}
		return;
	}

	// 既存の移動処理...
	switch (enemy.type) {
		case EnemyType::WALKER: {
			if (enemy.rect.x <= enemy.patrolMinX) {
				enemy.vel.x = enemy.speed;
				enemy.facingRight = true;
			}
			else if (enemy.rect.x >= enemy.patrolMaxX) {
				enemy.vel.x = -enemy.speed;
				enemy.facingRight = false;
			}
			enemy.pos.x += enemy.vel.x * dt;
			break;
		}
		case EnemyType::FLYER: {
			if (enemy.rect.y <= enemy.patrolMinY) {
				enemy.pos.y = enemy.patrolMinY;
				enemy.vel.y = fabs(enemy.speed);
			}
			else if (enemy.rect.y >= enemy.patrolMaxY) {
				enemy.pos.y = enemy.patrolMaxY;
				enemy.vel.y = -fabs(enemy.speed);
			}
			enemy.pos.y += enemy.vel.y * dt;
			break;
		}
		case EnemyType::JUMPCOPY: {
			if (enemy.rect.x <= enemy.patrolMinX) {
				enemy.vel.x = enemy.speed;
				enemy.facingRight = true;
			}
			else if (enemy.rect.x >= enemy.patrolMaxX) {
				enemy.vel.x = -enemy.speed;
				enemy.facingRight = false;
			}
			enemy.pos.x += enemy.vel.x * dt;

			const float gravity   = 1600.0f;
			const float jumpSpeed = 830.0f;
			const float groundY   = enemy.patrolMinY;
			const float eps       = 0.5f;

			const bool onGround = (enemy.pos.y >= groundY - eps);
			if (onGround) {
				enemy.pos.y = groundY;
				if (enemy.vel.y > 0.0f) enemy.vel.y = 0.0f;
			}

			const bool nearPlayer = playerSence(player, enemy, 220.0f);
			const bool playerJumpPressed = (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP));

			if (nearPlayer && onGround && playerJumpPressed) {
				enemy.vel.y = -jumpSpeed;
			}

			enemy.vel.y += gravity * dt;
			enemy.pos.y += enemy.vel.y * dt;

			if (enemy.pos.y > groundY) {
				enemy.pos.y = groundY;
				enemy.vel.y = 0.0f;
			}
			break;
		}
	}

	enemy.rect.x = enemy.pos.x;
	enemy.rect.y = enemy.pos.y;

	// isHit の間は少し表示してから消す例
	if (enemy.hp <= 0) {
		if (enemy.isHit) {
			if (enemy.timer >= 0.15f) {
				enemy.isActive = false;
			}
		}
		else {
			enemy.isActive = false;
		}
	}
}

void EnemyDraw(const Enemy& enemy) {
	if (!enemy.isActive) return;

	const Texture2D tex =
		(enemy.isStomped && enemy.stompTexture.id != 0) ? enemy.stompTexture :
		(enemy.isHit && enemy.hitTexture.id != 0) ? enemy.hitTexture :
		enemy.texture;

	if (tex.id == 0) {
		switch (enemy.type) {
		case EnemyType::WALKER:   DrawRectangleRec(enemy.rect, RED); break;
		case EnemyType::FLYER:    DrawRectangleRec(enemy.rect, BLUE); break;
		case EnemyType::JUMPCOPY: DrawRectangleRec(enemy.rect, MAROON); break;
		default:                  DrawRectangleRec(enemy.rect, GRAY); break;
		}
		return;
	}

	Rectangle src = {
		0.0f,
		0.0f,
		(float)tex.width,
		(float)tex.height
	};

	if (!enemy.facingRight) {
		src.x = (float)tex.width;
		src.width = -(float)tex.width;
	}

	const float centerX = enemy.rect.x + enemy.rect.width * 0.5f;
	const float centerY = enemy.rect.y + enemy.rect.height * 0.5f;

	Rectangle dst = {
		centerX,
		centerY + ENEMY_DRAW_Y_OFFSET,
		enemy.rect.width * ENEMY_DRAW_SCALE,
		enemy.rect.height * ENEMY_DRAW_SCALE
	};

	Vector2 origin = { dst.width * 0.5f, dst.height * 0.5f };
	DrawTexturePro(tex, src, dst, origin, 0.0f, WHITE);
}
void EnemyReset(Enemy& enemy) {
    enemy.isActive = false;
    enemy.isHit = false;
    enemy.isStomped = false;
    enemy.deathTimer = 0.0f;
    enemy.hp = 0;
    enemy.vel = { 0.0f, 0.0f };
}
