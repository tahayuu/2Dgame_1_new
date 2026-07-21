// ─────────────────────────────────────────────────────
//  Player.cpp  ── 「プレイヤーはこっちでやる」
//
//  main.cpp の PLAYING ブロック (旧 537〜920 行) を移動
// ─────────────────────────────────────────────────────
#include "Player.h"
#include "Stage.h"
#include "StageCollision.h"
#include "StageHazard.h"
#include "EnemyManager.h"
#include "ItemManager.h"
#include "OjisanVisual.h"
#include "ChengeStage.h"
#include "AudioManager.h"
#include"StageEventChanger.h"
#include <cmath>


// ─────────────────────────────────────────────────────
//  入力判定
// ─────────────────────────────────────────────────────
static bool IsJumpKeyPressed(int jumpMode) {
    switch (jumpMode) {
    case 1:
        // Nキーのみ
        return IsKeyPressed(KEY_N);

    case 0:
    default:
        // 通常操作
        return
            IsKeyPressed(KEY_SPACE) ||
            IsKeyPressed(KEY_W) ||
            IsKeyPressed(KEY_UP);
    }
}

// ─────────────────────────────────────────────────────
//  初期化・後処理
// ─────────────────────────────────────────────────────
// 目的: 描画素材(PlayerVisual)を読み込み、更新開始前の状態を作る。
void PlayerStateInit(PlayerState& ps)
{
    PlayerVisualLoad(ps.visual);
}

void PlayerStateUnload(PlayerState& ps)
{
    PlayerVisualUnload(ps.visual);
}

// ─────────────────────────────────────────────────────
//  毎フレーム更新
// ─────────────────────────────────────────────────────
// 目的: 入力・物理・ギミック・敵/アイテム接触を統合してプレイヤー状態を確定する。
// 入力: ステージ状態、各マネージャ、カメラ、経過時間。
// 出力: ps の位置/速度/死亡フラグ/演出フラグが更新される。
// 注意: 戻り値を使わない設計のため、main側は pendingDeath 等を必ず参照する。
void PlayerStateUpdate(PlayerState& ps,
    Stage& stage,
    EnemyManager& em,
    ItemManager& im,
    OjisanVisual& ojisan,
    Camera2D& cam,
    float         dt,
    bool          isInvincible,
    AudioManager& audio)
{
    // 結果フラグをリセット
    ps.pendingDeath = false;
    ps.pendingEnterEditor = false;

    // ── V / F1 : エディタモードへ ──────────────────
    if (IsKeyPressed(KEY_V) || IsKeyPressed(KEY_F1)) {
        ps.pendingEnterEditor = true;
        return;
    }

    // ── ステージからのフラグを受け取る ─────────────
    if (stage.playerKnockedBack) {
        ps.isKnockedBack = true;
        ps.knockBackTimer = 0.0f;
        stage.playerKnockedBack = false;
    }
    if (stage.playerJumped) {
        ps.isjumped = true;
        ps.jumpTimer = 0.0f;
        stage.playerJumped = false;
    }
    if (stage.playerFired) {
        ps.isFired = true;
        ps.fireTimer = 0.0f;
        stage.playerFired = false;
    }
    if (stage.playerGrabbedByCrane && !ps.isCraneGrabbed) {
        ps.isCraneGrabbed = true;
    }

    // ── タイマー更新 ────────────────────────────────
    if (ps.isFired) {
        ps.fireTimer += dt;
        if (ps.fireTimer >= ps.fireDuration) { ps.isFired = false; ps.fireTimer = 0.0f; }
    }
    if (ps.isCraneLaunched) {
        ps.craneLaunchTimer += dt;
        if (ps.craneLaunchTimer >= ps.craneLaunchDuration) ps.isCraneLaunched = false;
    }
    if (ps.isjumped) {
        ps.jumpTimer += dt;
        if (ps.jumpTimer >= ps.jumpDuration) ps.isjumped = false;
    }
    if (ps.isKnockedBack) {
        ps.knockBackTimer += dt;
        if (ps.knockBackTimer >= ps.knockBackDuration) ps.isKnockedBack = false;
    }

    // クレーンにプレイヤー矩形を毎フレーム渡す
    for (int i = 0; i < stage.craneCount; i++)
        stage.cranes[i].playerRect = ps.rect;

    // ── ステージ・アイテム更新 ──────────────────────
    StageUpdate(stage, dt, im, cam);
    im.UpdateAll(dt, ps.rect, ps.velocity);
    im.ItemCollisionAll(ps.item, ps.rect, dt, ps.velocity);
    HazardUpdate(stage, ps.rect, dt);

    float moveSpeed = ps.baseMoveSpeed + im.GetSpeedBoost();
    float jumpSpeed = ps.baseJumpSpeed + im.GetJumpBoost();

    bool onIce = IsOnIcePlatform(stage, ps.rect);
    bool onMoveDown = IsOnMoveDownPlatform(stage, ps.rect);
    bool onMoveUp = IsOnMoveUpPlatform(stage, ps.rect);

    // ── 横移動の速度を決める ────────────────────────
    if (ps.isKnockedBack) {
        // 吹っ飛ばし中：速度を維持（何もしない）
    }
    else if (ps.isjumped) {
        // ジャンプ台：入力無視
    }
    else if (ps.isFired) {
        const float firedBoost = 120.0f;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
            ps.velocity.x += firedBoost * dt;
    }
    else if (ps.isCraneGrabbed) {
        // クレーン：入力無視
    }
    else if (onIce && ps.onGround) {
        // 氷床
        float inputDir = 0.0f;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) inputDir += 1.0f;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) inputDir -= 1.0f;
        if (inputDir != 0.0f)
            ps.velocity.x += inputDir * ps.iceAccel * dt;
        else
            ps.velocity.x *= (1.0f - ps.iceFriction);
        if (ps.velocity.x > ps.maxIceSpeed) ps.velocity.x = ps.maxIceSpeed;
        if (ps.velocity.x < -ps.maxIceSpeed) ps.velocity.x = -ps.maxIceSpeed;
        if (fabsf(ps.velocity.x) < 1.0f)     ps.velocity.x = 0.0f;
    }
    else if (onMoveDown && ps.onGround) {
        // 移動低下床
        float inputDir = 0.0f;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) inputDir += 1.0f;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) inputDir -= 1.0f;
        ps.velocity.x = (inputDir != 0.0f) ? ps.velocity.x + inputDir * ps.moveDownAccel * dt : 0.0f;
        if (ps.velocity.x > ps.maxMoveDownSpeed) ps.velocity.x = ps.maxMoveDownSpeed;
        if (ps.velocity.x < -ps.maxMoveDownSpeed) ps.velocity.x = -ps.maxMoveDownSpeed;
    }
    else if (onMoveUp && ps.onGround) {
        // 移動上昇床
        float inputDir = 0.0f;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) inputDir += 1.0f;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) inputDir -= 1.0f;
        ps.velocity.x = (inputDir != 0.0f) ? ps.velocity.x + inputDir * ps.moveUpAccel : 0.0f;
        if (ps.velocity.x > ps.maxMoveUpSpeed) ps.velocity.x = ps.maxMoveUpSpeed;
        if (ps.velocity.x < -ps.maxMoveUpSpeed) ps.velocity.x = -ps.maxMoveUpSpeed;
    }
    else {
        // 通常移動
        ps.velocity.x = 0.0f;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) ps.velocity.x += moveSpeed;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) ps.velocity.x -= moveSpeed;
    }

    // ── ジャンプ ────────────────────────────────────
	UpdateEventChangers(stage, ps.rect);// EventChangerのエリア判定を更新
    float gravDir = stage.gravityReversed ? -1.0f : 1.0f;
    if (ps.onGround) {
        if (stage.playerInElevator) {
			if (IsJumpKeyPressed(stage.currentJumpMode))// エレベーター内でジャンプ入力があった場合
                ps.velocity.y = -jumpSpeed * gravDir;
        }
        else {
			if (IsJumpKeyPressed(stage.currentJumpMode)) {// ジャンプ入力があった場合
                ps.velocity.y = -jumpSpeed * gravDir;
                ps.onGround = false;
                AudioPlaySfx(audio, SfxId::Jump);
            }
        }
    }

    // ── 重力・移動・衝突解決 ────────────────────────
    if (!stage.playerInBattery)
        ps.velocity.y += ps.gravity * gravDir * dt;

    ps.prevRect = ps.rect;

    ps.rect.x += ps.velocity.x * dt;
    StageResolveX(stage, ps.rect, ps.velocity, dt, ps.prevRect);

    if (!stage.playerInBattery) {
        ps.rect.y += ps.velocity.y * dt;
        ps.onGround = StageResolveY(stage, ps.prevRect, ps.rect, ps.velocity, ps.prevRect, dt);
    }

    // ── ビジュアル更新 ──────────────────────────────
    PlayerVisualUpdate(ps.visual, dt, ps.velocity, ps.onGround);
    bool hasJumpBoost = (im.GetJumpBoost() > 0.0f);
    PlayerParticleUpdate(ps.visual.jumpEffect, dt, ps.rect, hasJumpBoost);

    // ── スプリング（R キー） ─────────────────────────
    if (IsKeyPressed(KEY_R)) {
        if (stage.heldSpringIndex < 0) {
            // 近くのスプリングを拾う
            for (int i = 0; i < stage.springCount; i++) {
                if (!stage.springs[i].isActive) continue;
                Vector2 pc = { ps.rect.x + ps.rect.width / 2.0f,
                               ps.rect.y + ps.rect.height / 2.0f };
                Vector2 sc = { stage.springs[i].rect.x + stage.springs[i].rect.width / 2.0f,
                               stage.springs[i].rect.y + stage.springs[i].rect.height / 2.0f };
                float d = sqrtf((pc.x - sc.x) * (pc.x - sc.x) + (pc.y - sc.y) * (pc.y - sc.y));
                if (d < 100.0f) {
                    stage.heldSpringIndex = i;
                    stage.springs[i].isActive = false;
                    break;
                }
            }
        }
        else {
            // 目の前に置く
            int idx = stage.heldSpringIndex;
            if (idx >= 0 && idx < stage.springCount) {
                bool  facingRight = (ps.velocity.x >= 0.0f);
                float offsetX = facingRight
                    ? (ps.rect.width + 20.0f)
                    : (-stage.springs[idx].rect.width - 20.0f);
                stage.springs[idx].rect.x = ps.rect.x + offsetX;
                stage.springs[idx].rect.y = ps.rect.y + ps.rect.height
                    - stage.springs[idx].rect.height;
                stage.springs[idx].isActive = true;
                stage.heldSpringIndex = -1;
            }
        }
    }

    // ── 敵との当たり判定 ────────────────────────────
    em.EnemyCollisionAll(ps.rect, dt, ps.velocity);
    if (!isInvincible && em.playerTouched) {
        ps.pendingDeath = true;
        ps.lastDeathCause = DeathCause::ENEMY_WALKER;
        return;
    }
    em.UpdateAll(dt, ps.rect);

    // ── おじさんパンチ領域 ──────────────────────────
    /*for (int i = 0; i < stage.ojisanPunchAreaCount; i++) {
        if (!stage.ojisanPunchTriggered[i] &&
            CheckCollisionRecs(ps.rect, stage.ojisanPunchAreas[i]))
        {
     
            stage.ojisanPunchTriggered[i] = true;
            ps.ojisanPunchScreenPos = GetWorldToScreen2D({ ps.rect.x, ps.rect.y }, cam);
            ps.isOjisanPunchDeath = true;
            ps.gravityReversedAtDeath = stage.gravityReversed;
            ojisan.showPunch = true;
            

            AudioPlaySfx(audio, SfxId::Punch);
            ps.velocity = { 0.0f, 0.0f };
            ps.pendingDeath = true;
            ps.lastDeathCause = DeathCause::OJISAN_PUNCH;
            return;
        }
    }*/

    // ── ステージ機構の更新 ──────────────────────────
    MoveUpdateWithPlayrer(stage, ps.rect, ps.velocity, dt);
    ElevatorUpdate(stage, ps.rect, ps.velocity, dt);
    UpdateMagnet(stage, ps.rect, ps.velocity, dt, cam);

    // ── クレーン：位置をフックに固定 ────────────────
    if (ps.isCraneGrabbed) {
        for (int i = 0; i < stage.craneCount; i++) {
            auto& cr = stage.cranes[i];
            if (cr.state == CraneState::CARRYING || cr.state == CraneState::GRABBING) {
                float hookCX = cr.bodyRect.x + cr.bodyRect.width / 2.0f;
                float hookY = cr.ceilingY + cr.armLength;
                ps.rect.x = hookCX - ps.rect.width / 2.0f;
                ps.rect.y = hookY;
                ps.velocity = { 0.0f, 0.0f };
                break;
            }
        }
    }

    // ── クレーン死亡 ────────────────────────────────
    if (!isInvincible && stage.playerCraneKill) {
        stage.playerCraneKill = false;
        stage.playerGrabbedByCrane = false;
        ps.isCraneGrabbed = false;
        ps.pendingDeath = true;
        ps.lastDeathCause = DeathCause::TRAP;
        return;
    }

    // ── 死亡判定（デスブロック・トゲ） ──────────────
    bool hitDeathBlock = false;
    for (int i = 0; i < stage.deathBlockCount; i++) {
        if (CheckCollisionRecs(ps.rect, stage.deathBlocks[i])) {
            hitDeathBlock = true; break;
        }
    }
    if (!isInvincible && hitDeathBlock) {
        ps.pendingDeath = true; ps.lastDeathCause = DeathCause::TRAP;         return;
    }
    if (!isInvincible && StageHitRisingSpike(stage, ps.rect)) {
        ps.pendingDeath = true; ps.lastDeathCause = DeathCause::SPIKE_RISING; return;
    }
    if (!isInvincible && StageHitHazard(stage, ps.rect)) {
        ps.pendingDeath = true; ps.lastDeathCause = DeathCause::SPIKE;        return;
    }

    // ── コメントブロック ────────────────────────────
    for (int i = 0; i < stage.commentBlockCount; i++) {
        auto& cb = stage.commentBlocks[i];
        if (cb.cooldown > 0.0f) { cb.cooldown -= dt; continue; }
        if (!cb.triggered && CheckCollisionRecs(ps.rect, cb.rect)) {
            cb.triggered = true;
            cb.cooldown = cb.duration + 1.0f;
            ojisan.TriggerMessage(cb.message, cb.duration, nullptr, false);
        }
        if (cb.triggered && !CheckCollisionRecs(ps.rect, cb.rect)) {
            cb.triggered = false;
        }
    }
}

// ─────────────────────────────────────────────────────
//  描画（ワールド座標）BeginMode2D の中で呼ぶ
// ─────────────────────────────────────────────────────
// 目的: ゲーム世界内に存在するプレイヤー表示を描画する。
void PlayerStateDrawWorld(const PlayerState& ps,
    float              editorExitInvTimer,
    bool               isDeadScreen)
{
    // 無敵中は点滅
    bool showPlayer = (editorExitInvTimer <= 0.0f) ||
        ((int)(editorExitInvTimer * 10.0f) % 2 == 0);

    // おじさんパンチ死亡中は通常スプライトを非表示
    if (showPlayer && !(ps.isOjisanPunchDeath && isDeadScreen)) {
        PlayerVisualDraw(ps.visual, ps.rect, ps.velocity, ps.gravityReversedAtDeath);
    }

    PlayerParticleDraw(ps.visual.jumpEffect);
}

// ─────────────────────────────────────────────────────
//  描画（スクリーン座標）EndMode2D の後で呼ぶ
//  おじさんパンチ死亡演出
// ─────────────────────────────────────────────────────
void PlayerStateDrawScreen(const PlayerState& ps,
    bool               isDeadScreen,
    const Texture2D& punchEffect, AudioManager& audio, SfxId id)
{
    if (!isDeadScreen || !ps.isOjisanPunchDeath) return;
 
    // 死亡時の画面座標にスプライトを固定表示
    Rectangle stuckPlayer = {
        ps.ojisanPunchScreenPos.x,
        ps.ojisanPunchScreenPos.y,
        ps.rect.width,
        ps.rect.height
    };
    PlayerVisualDraw(ps.visual, stuckPlayer, ps.velocity, ps.gravityReversedAtDeath);

    // パンチエフェクトを重ねる
    if (punchEffect.id != 0) {
   
        constexpr float scale = 0.3f;
        Rectangle src = { 0.0f, 0.0f, (float)punchEffect.width, (float)punchEffect.height };
        Rectangle dst = {
            ps.ojisanPunchScreenPos.x + ps.rect.width * 0.5f,
            ps.ojisanPunchScreenPos.y + ps.rect.height * 0.5f,
            punchEffect.width * scale,
            punchEffect.height * scale
        };
        Vector2 origin = { dst.width * 0.5f, dst.height * 0.5f };
        DrawTexturePro(punchEffect, src, dst, origin, 0.0f, WHITE);
    }
}