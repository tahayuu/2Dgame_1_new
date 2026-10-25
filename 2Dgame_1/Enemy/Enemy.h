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

	// --------------------------------------------------------------
	// 公開アクセサ
	// ---------------------------------------------------------------
	// 旧実装ではフィールドを直接publicにしてStageInit/Editor/mainから
	// 参照・書き換えしていたが、カプセル化のため以下のアクセサ経由に変更する。
	// 既存仕様(挙動)は変えず、書き換え方法だけをメソッド呼び出しに揃えている。
	EnemyType GetType() const { return type; }
	bool IsActive() const { return isActive; }
	bool IsHit() const { return isHit; }
	void SetHit(bool hit) { isHit = hit; }
	const std::string& GetDialogKey() const { return dialogKey; }
	void SetDialogKey(const std::string& key) { dialogKey = key; }
	float GetSpeed() const { return speed; }
	void SetSpeed(float value) { speed = value; }
	float GetHp() const { return hp; }
	void SetHp(float value) { hp = value; }
	// Editor側で配置位置からパトロール範囲を計算して設定するために使う。
	void SetPatrolRangeX(float minX, float maxX) { patrolMinX = minX; patrolMaxX = maxX; }
	void SetPatrolRangeY(float minY, float maxY) { patrolMinY = minY; patrolMaxY = maxY; }

protected:
	// 内部実装(派生クラスやEnemy.cpp内の補助関数)からのみ参照するフィールド。
	// EnemyManagerおよびEnemy.cpp内の各種ヘルパー関数は、Enemyと密結合な
	// 実装詳細のためfriend指定で直接アクセスを許可している。
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

	void InitializeCommon(EnemyType enemyType, Vector2 position);
	virtual Vector2 GetCollisionBounce() const { return { 100.0f, 200.0f }; }
	virtual bool RespondsToPlayerCollision() const { return true; }

	// Enemy.cpp内の補助関数(当たり判定・描画・死亡処理)は、Enemyの
	// 実装の一部として protected フィールドへ直接アクセスする。
	friend class EnemyManager;
	friend bool CheckOverlapXEnemy(const Rectangle& player, const Enemy& enemy);
	friend bool CheckOverlapYEnemy(const Rectangle& player, const Enemy& enemy);
	friend bool CheckNealapYEnemy(const Rectangle& player, const Enemy& enemy, float tolerance);
	friend void EnemyCollisionErase(Enemy& enemy, const Rectangle& player, float dt, Vector2& velocity, float Y, float X, AudioManager& audio);
	friend bool playerSence(const Rectangle& player, const Enemy& enemy, float rangeX);
	friend bool UpdateStompDeath(Enemy& enemy, float dt);
	friend void UpdateHpDeath(Enemy& enemy);
	friend void EnemyDraw(const Enemy& enemy);
	friend void EnemyReset(Enemy& enemy);
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
