#pragma once
#include "raylib.h"
#include "StageTypes.h"
#include"EnemyManager.h"
#include"ItemManager.h"

// 初期化
void StageInit_1(Stage& stage, EnemyManager& enemyManager);
void StageInit_2(Stage& stage, EnemyManager& enemyManager, ItemManager& itemManager);
void StageInit_3(Stage& stage, EnemyManager& enemyManager, ItemManager& itemManager);
void StageInit_choseStage(Stage& stage, EnemyManager& enemyManager);
void StageInit_debug(Stage& stage, EnemyManager& enemyManager,ItemManager& itemManager);

// 更新
void StageUpdate(Stage& stage, float dt,ItemManager& itemManager,Camera2D camera);


// リセット
void StageReset(Stage& stage);

// ステージ切り替え前の全データクリア
void StageClear(Stage& stage);

// トゲ描画関数（宣言）
void DrawSpikes(Rectangle h, float spikeW);
// カーソル追従床更新
void UpdateMagnet(Stage& stage, Rectangle& player, Vector2& velocity,float dt);
// 描画
void StageDraw(const Stage& stage, float spikeW);
