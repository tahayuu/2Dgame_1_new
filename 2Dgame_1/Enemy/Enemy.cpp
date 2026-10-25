#include"Enemy.h"
#include"raylib.h"
#include"StageCollision.h"
#include<cmath>

// ================================================================
// Enemy.cpp の役割
// ---------------------------------------------------------------
// ・敵の初期化、接触判定、移動更新、描画を実装する。
// ・側面衝突と踏みつけを分離し、見た目切替と死亡遷移を管理する。
// ================================================================

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

// 目的: 敵種別ごとの画像と被弾/踏みつけ画像を読み込む。
// 注意: 二重読み込みを防ぐため texturesLoaded でガードする。
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

static void EnemyCollisionErase(Enemy& enemy, const Rectangle& player, float dt, Vector2& velocity, float Y, float X, AudioManager& audio) {
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
			AudioPlaySfx(audio,SfxId::hitting);
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

// 目的: 踏みつけ後の死亡タイマーを共通更新し、移動を止めるか返す。
static bool UpdateStompDeath(Enemy& enemy, float dt) {
	if (!enemy.isStomped) return false;
	enemy.deathTimer += dt;
	if (enemy.deathTimer >= 0.15f) {
		enemy.isActive = false;
	}
	return true;
}

// 目的: HPが0になった敵を、既存の被弾表示時間に従って非アクティブにする。
static void UpdateHpDeath(Enemy& enemy) {
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

//==========================
// 公開関数の定義
//==========================
// 目的: 各派生クラスが共通して使う初期状態を設定する。
void Enemy::InitializeCommon(EnemyType enemyType, Vector2 spawnPos) {
	type = enemyType;
	pos = spawnPos;
	vel = { 0.0f, 0.0f };
	isActive = true;
	speed = 100.0f;
	hp = 1.0f;
	PlayerTouch = false;
	isHit = false;
	isStomped = false;
	touchedFromSide = false;
	timer = 0.0f;
	facingRight = true;
	isDying = false;
	deathTimer = 0.0f;
	animTimer = 0.0f;
	currentFrame = 0;
	patrolMinX = spawnPos.x;
	patrolMaxX = spawnPos.x;
	patrolMinY = spawnPos.y;
	patrolMaxY = spawnPos.y;
	rect = { spawnPos.x, spawnPos.y, ENEMY_HITBOX_W, ENEMY_HITBOX_H };
	this->hitTexture = ::hitTexture;
	this->stompTexture = ::stompTexture;
}

void WalkerEnemy::Initialize(Vector2 position) {
	InitializeCommon(EnemyType::WALKER, position);
	vel = { 100.0f, 100.0f };
	patrolMinX = position.x - 100.0f;
	patrolMaxX = position.x + 100.0f;
	texture = walkerTexture;
	fallbackColor = RED;
}

void FlyerEnemy::Initialize(Vector2 position) {
	InitializeCommon(EnemyType::FLYER, position);
	vel = { 100.0f, 100.0f };
	patrolMinY = position.y - 100.0f;
	patrolMaxY = position.y + 100.0f;
	texture = flyerTexture;
	fallbackColor = BLUE;
}

void JumpCopyEnemy::Initialize(Vector2 position) {
	InitializeCommon(EnemyType::JUMPCOPY, position);
	vel = { 100.0f, 100.0f };
	patrolMinX = position.x - 100.0f;
	patrolMaxX = position.x + 100.0f;
	patrolMinY = position.y;
	texture = jumpcopyTexture;
	fallbackColor = MAROON;
}

void ShooterEnemy::Initialize(Vector2 position) {
	InitializeCommon(EnemyType::SHOOTER, position);
	fallbackColor = GRAY;
}

// 目的: Walkerの左右巡回を更新し、共通の踏みつけ/死亡状態を維持する。
void WalkerEnemy::Update(float dt, const Rectangle& player) {
	(void)player;
	if (!isActive) return;

	if (UpdateStompDeath(*this, dt)) return;

	if (rect.x <= patrolMinX) {
		vel.x = speed;
		facingRight = true;
	}
	else if (rect.x >= patrolMaxX) {
		vel.x = -speed;
		facingRight = false;
	}
	pos.x += vel.x * dt;

	rect.x = pos.x;
	rect.y = pos.y;
	UpdateHpDeath(*this);
}

// 目的: Flyerの上下巡回と共通の死亡状態を更新する。
void FlyerEnemy::Update(float dt, const Rectangle& player) {
	(void)player;
	if (!isActive) return;
	if (UpdateStompDeath(*this, dt)) return;

	if (rect.y <= patrolMinY) {
		pos.y = patrolMinY;
		vel.y = fabs(speed);
	}
	else if (rect.y >= patrolMaxY) {
		pos.y = patrolMaxY;
		vel.y = -fabs(speed);
	}
	pos.y += vel.y * dt;

	rect.x = pos.x;
	rect.y = pos.y;
	UpdateHpDeath(*this);
}

// 目的: JumpCopyの巡回、重力、プレイヤー入力を使ったジャンプ模倣を更新する。
void JumpCopyEnemy::Update(float dt, const Rectangle& player) {
	if (!isActive) return;
	if (UpdateStompDeath(*this, dt)) return;

	if (rect.x <= patrolMinX) {
		vel.x = speed;
		facingRight = true;
	}
	else if (rect.x >= patrolMaxX) {
		vel.x = -speed;
		facingRight = false;
	}
	pos.x += vel.x * dt;

	const float gravity = 1600.0f;
	const float jumpSpeed = 830.0f;
	const float groundY = patrolMinY;
	const float eps = 0.5f;

	const bool onGround = (pos.y >= groundY - eps);
	if (onGround) {
		pos.y = groundY;
		if (vel.y > 0.0f) vel.y = 0.0f;
	}

	const bool nearPlayer = playerSence(player, *this, 220.0f);
	const bool playerJumpPressed = (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP));

	if (nearPlayer && onGround && playerJumpPressed) {
		vel.y = -jumpSpeed;
	}

	vel.y += gravity * dt;
	pos.y += vel.y * dt;

	if (pos.y > groundY) {
		pos.y = groundY;
		vel.y = 0.0f;
	}

	rect.x = pos.x;
	rect.y = pos.y;
	UpdateHpDeath(*this);
}

// 目的: 現在未実装のShooter AIに新しい動作を加えず、共通死亡状態のみ更新する。
void ShooterEnemy::Update(float dt, const Rectangle& player) {
	(void)dt;
	(void)player;
	if (!isActive) return;
	if (UpdateStompDeath(*this, dt)) return;

	rect.x = pos.x;
	rect.y = pos.y;
	UpdateHpDeath(*this);
}

// 目的: enemy の状態に応じて通常/被弾/踏みつけテクスチャを描き分ける。
void EnemyDraw(const Enemy& enemy) {
	if (!enemy.isActive) return;

	const Texture2D tex =
		(enemy.isStomped && enemy.stompTexture.id != 0) ? enemy.stompTexture :
		(enemy.isHit && enemy.hitTexture.id != 0) ? enemy.hitTexture :
		enemy.texture;

	if (tex.id == 0) {
		DrawRectangleRec(enemy.rect, enemy.fallbackColor);
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

// 目的: 全敵に共通する接触判定を行い、種別ごとの反発値だけを仮想特性から取得する。
void Enemy::OnCollision(const Rectangle& player, float dt, Vector2& velocity, AudioManager& audio) {
	if (!isActive) return;
	timer += dt;
	if (!RespondsToPlayerCollision()) return;

	const Vector2 bounce = GetCollisionBounce();
	EnemyCollisionErase(*this, player, dt, velocity, bounce.y, bounce.x, audio);
}

// 目的: 全敵に共通する描画処理を既存関数へ委譲する。
void Enemy::Draw() const {
	EnemyDraw(*this);
}

// 目的: 敵の稼働状態を既存のリセット仕様に従って初期化する。
void Enemy::Reset() {
	EnemyReset(*this);
}

// 目的: 敵種別に対応する具象クラスを生成し、クラスごとの初期値を設定する。
std::unique_ptr<Enemy> CreateEnemy(EnemyType type, Vector2 position) {
	std::unique_ptr<Enemy> enemy;
	switch (type) {
	case EnemyType::WALKER:
		enemy = std::make_unique<WalkerEnemy>();
		break;
	case EnemyType::FLYER:
		enemy = std::make_unique<FlyerEnemy>();
		break;
	case EnemyType::SHOOTER:
		enemy = std::make_unique<ShooterEnemy>();
		break;
	case EnemyType::JUMPCOPY:
		enemy = std::make_unique<JumpCopyEnemy>();
		break;
	default:
		return nullptr;
	}
	enemy->Initialize(position);
	return enemy;
}
