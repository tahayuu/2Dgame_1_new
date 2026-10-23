#include "Stage.h"
#include "StageInit_4.h"
#include "StageInit_1.h"
#include "StageInit_2.h"
#include "StageInit_3.h"
#include "StageInit_2222.h"
#include "StageInit_choseStage.h"
#include"ItemManager.h"

// ================================================================
// ChengeStage.cpp の役割
// ---------------------------------------------------------------
// ・ステージ番号と StageInit_* 関数の対応表を実装する。
// ・切替時に既存ステージをクリアしてから再初期化する。
// ================================================================

// 目的: ステージ切替時に旧データを初期化し、目的ステージの配置を再構築する。
void ChangeStage(Stage& stage, int stageNo,EnemyManager& enemyManager, ItemManager& itemManager) {
    StageClear(stage);
    switch (stageNo) {
    case 1: StageInit_1(stage, enemyManager,itemManager); break;
    case 2: StageInit_2(stage, enemyManager,itemManager); break;
    case 3: StageInit_3(stage, enemyManager,itemManager); break;
    case 4: StageInit_4(stage, enemyManager,itemManager); break;
    case 99: StageInit_debug(stage,enemyManager,itemManager); break;
    case 0: StageInit_choseStage(stage, enemyManager); break;
        // 必要に応じて追加
    }
}
