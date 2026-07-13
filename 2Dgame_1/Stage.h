#pragma once
#include "raylib.h"
#include "StageTypes.h"
#include"EnemyManager.h"
#include"ItemManager.h"

// ================================================================
// Stage.h の役割
// ---------------------------------------------------------------
// ・ステージ初期化/更新/リセットの入口関数を宣言する。
// ・Stage 実体は StageTypes.h の struct Stage で定義する。
// ================================================================

// 初期化
void StageInit_1(Stage& stage, EnemyManager& enemyManager);
void StageInit_2(Stage& stage, EnemyManager& enemyManager, ItemManager& itemManager);
void StageInit_3(Stage& stage, EnemyManager& enemyManager, ItemManager& itemManager);
void StageInit_choseStage(Stage& stage, EnemyManager& enemyManager);
void StageInit_debug(Stage& stage, EnemyManager& enemyManager,ItemManager& itemManager);

// 更新
// 目的: ギミック状態を1フレーム分更新する。
void StageUpdate(Stage& stage, float dt,ItemManager& itemManager,Camera2D camera);


// リセット
// 目的: ステージ状態を初期化済みデータへ戻す。
void StageReset(Stage& stage);

// ステージ切り替え前の全データクリア
// 目的: ステージ切替前に配列/カウントを安全にクリアする。
void StageClear(Stage& stage);

// トゲ描画関数（宣言）
// 目的: トゲ描画ヘルパー（宣言のみ）。
void DrawSpikes(Rectangle h, float spikeW);
// カーソル追従床更新
// 目的: 磁石ギミック（引き寄せ）の更新。
void UpdateMagnet(Stage& stage, Rectangle& player, Vector2& velocity,float dt, Camera2D camera);
// 描画
// 目的: ステージ描画関数（宣言のみ）。
void StageDraw(const Stage& stage, float spikeW);
