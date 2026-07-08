#pragma once
#include"raylib.h"
// 前方宣言
struct Stage;

// ================================================================
// StageCollision.h の役割
// ---------------------------------------------------------------
// ・プレイヤーとステージギミックの衝突解決APIを公開する。
// ・X/Y分離で解決し、押し戻しや着地判定を安定させる。
// ================================================================

//===========================================
// プレイヤーの衝突解決
//===========================================

// X方向（壁）の衝突解決
// 目的: 横方向(X)の衝突を解決する。
// 入力: stage、現在player、速度、前フレームplayer。
// 出力: player.x / velocity.x が補正される。
void StageResolveX(Stage& stage, Rectangle& player, Vector2& velocity,float dt,Rectangle& prevPlayer);

// Y方向（床・天井）の衝突解決（着地していればtrue）
// 目的: 縦方向(Y)の衝突を解決する。
// 入力: stage、前フレームplayer、現在player、速度。
// 出力: player.y / velocity.y を補正し、着地したら true を返す。
bool StageResolveY(Stage& stage, const Rectangle& prevPlayer, Rectangle& player, Vector2& velocity,Rectangle& prev,float dt);


// 目的: 動く床の移動量をプレイヤーへ伝搬する。
void MoveUpdateWithPlayrer(Stage& stage, Rectangle& player, Vector2& velocity, float dt);
// 目的: それぞれの床タイプ上にプレイヤーが立っているかを判定する。
bool IsOnIcePlatform(const Stage& stage, const Rectangle player);
bool IsOnMoveDownPlatform(const Stage& stage, const Rectangle player);
bool IsOnMoveUpPlatform(const Stage& stage, const Rectangle player);
// 目的: エレベーターの上下移動とプレイヤー追従を更新する。
void ElevatorUpdate(Stage& stage, Rectangle& player, Vector2& velocity, float dt);