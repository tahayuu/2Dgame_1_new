#pragma once
#include "raylib.h"
#include "PlayerVisual.h"
#include "GameEvents.h"
#include "AudioManager.h"
#include "Item.h"


// ================================================================
// Player.h の役割
// ---------------------------------------------------------------
// ・プレイヤー更新に必要な状態(PlayerState)と公開APIを定義する。
// ・移動/死亡/演出フラグを1か所に集約し、main側は結果フラグを参照する設計。
// ================================================================

struct Stage;
struct EnemyManager;
struct ItemManager;
class OjisanVisual;

// プレイヤーに関するデータをひとまとめ
struct PlayerState {
    Rectangle rect = { 100.0f, 500.0f, 60.0f, 70.0f };
    Rectangle prevRect = { 100.0f, 500.0f, 60.0f, 70.0f };
    Vector2 velocity = { 0.0f, 0.0f };
    bool onGround = false;
    bool onTop = false;

    bool isKnockedBack = false;
    float knockBackTimer = 0.0f;
    float knockBackDuration = 0.5f;

    bool isjumped = false;
    float jumpTimer = 0.0f;
    float jumpDuration = 0.5f;

    bool isFired = false;
    float fireTimer = 0.0f;
    float fireDuration = 0.7f;

    bool isCraneLaunched = false;
    float craneLaunchTimer = 0.0f;
    float craneLaunchDuration = 0.6f;
    bool isCraneGrabbed = false;

    float baseMoveSpeed = 300.0f;
    float baseJumpSpeed = 830.0f;
    // EventChangerのエリア判定
 
	float gravity = 1600.0f;// 重力加速度

    float iceAccel = 400.0f;
    float iceFriction = 0.02f;
    float maxIceSpeed = 500.0f;
    float moveDownAccel = 400.0f;
    float maxMoveDownSpeed = 300.0f;
    float moveUpAccel = 1000.0f;
    float maxMoveUpSpeed = 1500.0f;

    Vector2 respawn = { 100.0f, 500.0f };
    int deaths = 0;

    bool isOjisanPunchDeath = false;
    Vector2 ojisanPunchScreenPos = { 0.0f, 0.0f };
    bool gravityReversedAtDeath = false;

    bool pendingDeath = false;
    bool pendingEnterEditor = false;
    DeathCause lastDeathCause = DeathCause::FALL;
    
	bool isPlayingDeathAnimation = false;// 死亡アニメーション再生中か
	float deathAnimationTimer = 0.0f;    // 死亡アニメーションの経過時間
	float deathAnimationDuration = 1.3f; // 死亡アニメーションの総時間

    Item item{};
    PlayerVisual visual{};
};

// 目的: プレイヤー描画資源を初期化する。
void PlayerStateInit(PlayerState& ps);
// 目的: プレイヤー描画資源を解放する。
void PlayerStateUnload(PlayerState& ps);

// 目的: 1フレーム分のプレイヤー操作・物理・死亡判定を更新する。
// 入力: stage/enemy/item/camera と入力状態。
// 出力: pendingDeath / pendingEnterEditor などの結果フラグを更新する。
void PlayerStateUpdate(PlayerState& ps,
	Stage& stage,
	EnemyManager& em,
	ItemManager& im,
	OjisanVisual& ojisan,
	Camera2D& cam,
	float dt,
	bool isInvincible,
	AudioManager& audio
	);

// 目的: ワールド座標系でプレイヤー本体と粒子を描画する。
void PlayerStateDrawWorld(const PlayerState& ps,
	float editorExitInvTimer,
	bool isDeadScreen);

// 目的: スクリーン座標系の死亡演出（おじさんパンチ）を描画する。
void PlayerStateDrawScreen(const PlayerState& ps,
	bool isDeadScreen,
	const Texture2D& punchEffect, AudioManager& a, SfxId id);
