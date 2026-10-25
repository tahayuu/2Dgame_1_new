#pragma once
#include "raylib.h"
#include <string>
#include <memory>
#include "AudioManager.h"

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

class Enemy {
public:
	virtual ~Enemy() = default;

	// 種類ごとに異なるAI更新を派生クラスで実装する。
	virtual void Update(float dt, const Rectangle& player) = 0;
	// 生成時の共通状態と、敵種別ごとの初期値を設定する。
	virtual void Initialize(Vector2 position) = 0;
	// 当たり判定・描画・リセットは全敵で共通の既定処理を使う。
	virtual void OnCollision(const Rectangle& player, float dt, Vector2& velocity, AudioManager& audio);
	virtual void Draw() const;
	virtual void Reset();
	virtual Rectangle GetRect() const { return rect; }

	// 既存のStageInit/Editor/mainから直接参照されるため、段階移行中は公開を維持する。
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
	Color fallbackColor = GRAY;
	float animTimer = 0.0f;
	int currentFrame = 0; // 現在のアニメーションフレーム
	bool facingRight = true; // 敵の向き
	bool isDying = false;
	float deathTimer = 0.0f;  // 追加: 踏みつけ後の表示時間

protected:
	void InitializeCommon(EnemyType enemyType, Vector2 position);
	virtual Vector2 GetCollisionBounce() const { return { 100.0f, 200.0f }; }
	virtual bool RespondsToPlayerCollision() const { return true; }
};

// 目的: WALKER固有の左右巡回AIを実装する。
class WalkerEnemy final : public Enemy {
public:
	void Initialize(Vector2 position) override;
	void Update(float dt, const Rectangle& player) override;
};

// 目的: Flyer固有の上下巡回AIを実装する。
class FlyerEnemy final : public Enemy {
public:
	void Initialize(Vector2 position) override;
	void Update(float dt, const Rectangle& player) override;
};

// 目的: JumpCopy固有の左右巡回とプレイヤー入力模倣AIを実装する。
class JumpCopyEnemy final : public Enemy {
public:
	void Initialize(Vector2 position) override;
	void Update(float dt, const Rectangle& player) override;

protected:
	Vector2 GetCollisionBounce() const override { return { 150.0f, 300.0f }; }
};

// 目的: Shooterの既存状態を保つ専用クラス。現在のAI・射撃処理は未実装。
class ShooterEnemy final : public Enemy {
public:
	void Initialize(Vector2 position) override;
	void Update(float dt, const Rectangle& player) override;

protected:
	bool RespondsToPlayerCollision() const override { return false; }
};

// 目的: 敵の種類に対応する派生クラスを生成する。
std::unique_ptr<Enemy> CreateEnemy(EnemyType type, Vector2 position);

// 目的: 敵の現在状態に応じた見た目を描画する。
void EnemyDraw(const Enemy& enemy);
void EnemyReset(Enemy& enemy);
// 目的: 敵テクスチャを一括読み込みする。
void EnemyLoadTextures();
// 目的: 敵テクスチャを一括解放する。
void EnemyUnloadTextures();
