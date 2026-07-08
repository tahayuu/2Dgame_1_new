#pragma once
#include "Stage.h"
#include "StageInit_deback.h"
#include "StageInit_1.h"
#include "StageInit_2.h"
#include "StageInit_3.h"
#include "StageInit_choseStage.h"

// ================================================================
// ChengeStage.h の役割
// ---------------------------------------------------------------
// ・ステージ番号から対応する StageInit_* を呼び分けるAPIを宣言する。
// ================================================================

// 目的: 現在ステージをクリア後、stageNoに応じた初期化関数へ切り替える。
void ChangeStage(Stage& stage, int stageNo, EnemyManager& enemyManager, ItemManager& itemManager);
