#pragma once
#include"raylib.h"
// 前方宣言
struct Stage;

//===========================================
// プレイヤーの衝突解決
//===========================================

// X方向（壁）の衝突解決
void StageResolveX(Stage& stage, Rectangle& player, Vector2& velocity,float dt,Rectangle& prevPlayer);

// Y方向（床・天井）の衝突解決（着地していればtrue）
bool StageResolveY(Stage& stage, const Rectangle& prevPlayer, Rectangle& player, Vector2& velocity,Rectangle& prev,float dt);


void MoveUpdateWithPlayrer(Stage& stage, Rectangle& player, Vector2& velocity, float dt);
bool IsOnIcePlatform(const Stage& stage, const Rectangle player);
bool IsOnMoveDownPlatform(const Stage& stage, const Rectangle player);
bool IsOnMoveUpPlatform(const Stage& stage, const Rectangle player);
void ElevatorUpdate(Stage& stage, Rectangle& player, Vector2& velocity, float dt);