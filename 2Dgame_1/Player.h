#pragma once
#include "raylib.h"
#include "PlayerVisual.h"
#include "GameEvents.h"
#include "AudioManager.h"
#include "Item.h"

struct Stage;
struct EnemyManager;
struct ItemManager;
struct OjisanVisual;

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
    float gravity = 1600.0f;

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

    Item item{};
    PlayerVisual visual{};
};

void PlayerStateInit(PlayerState& ps);
void PlayerStateUnload(PlayerState& ps);

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

void PlayerStateDrawWorld(const PlayerState& ps,
    float editorExitInvTimer,
    bool isDeadScreen);

void PlayerStateDrawScreen(const PlayerState& ps,
    bool isDeadScreen,
    const Texture2D& punchEffect);
