#pragma once
#include "raylib.h"

// 前方宣言
struct Stage;

// ================================================================
// StageHazard.h の役割
// ---------------------------------------------------------------
// ・危険ギミックの当たり判定APIと更新APIを公開する。
// ・Player側は StageHitHazard/HazardUpdate を通して一括利用する。
// ================================================================

//===========================================
// ハザード（危険物）の当たり判定（個別）
//===========================================

bool StageHitStaticSpike    (const Stage& stage, const Rectangle& player); // 固定トゲ
bool StageHitUpSpike        (const Stage& stage, const Rectangle& player); // 上に動くトゲ
bool StageHitRisingSpike    (const Stage& stage, const Rectangle& player); // 上に動くトゲ（Y拡張）
bool StageHitDownSpike      (const Stage& stage, const Rectangle& player); // 下に動くトゲ
bool StageHitRightSpike     (const Stage& stage, const Rectangle& player); // 右に動くトゲ（X拡張）
bool StageHitRightSpikeY    (const Stage& stage, const Rectangle& player); // 右に動くトゲ（Y一致）
bool StageHitTrackingSpike  (const Stage& stage, const Rectangle& player); // 追尾トゲ
bool StageHitFallingText    (const Stage& stage, const Rectangle& player); // 落下文字
bool StageHitRotatingBall   (const Stage& stage, const Rectangle& player); // 回転鉄球
bool StageHitMoveRotatingBall(const Stage& stage, const Rectangle& player);// 動く回転鉄球
bool StageHitRollingBall    (const Stage& stage, const Rectangle& player); // 転がる鉄球

//===========================================
// まとめて判定（既存コードとの互換用）
//===========================================
// 目的: 全ハザード判定をまとめて実行する統合入口。
bool StageHitHazard(const Stage& stage, const Rectangle& player);

// 透明ブロックに当たったか
// 目的: 透明クリアブロックへの接触判定。
bool clearCheck(const Stage& stage, const Rectangle& player);

//===========================================
// ハザードの起動判定・更新
//===========================================
void UpMoveHazard           (Stage& stage, const Rectangle& player, float dt);
void UpMoveHazardExtY       (Stage& stage, const Rectangle& player, float dt);
void MoveHazarardNearRight  (Stage& stage, const Rectangle& player, float dt);
void TrackingHazardMove     (Stage& stage, const Rectangle& player, float dt);
void StageSenseFallingTexts (Stage& stage, const Rectangle& player, float dt);
// 目的: 危険ギミックの起動判定と移動更新を1フレーム分進める。
void HazardUpdate           (Stage& stage, const Rectangle& player, float dt);
// 目的: 危険ギミックをステージ初期状態へ戻す。
void HazardReset            (Stage& stage);
