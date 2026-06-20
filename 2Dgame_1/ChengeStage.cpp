#include "Stage.h"
#include "StageInit_deback.h"
#include "StageInit_1.h"
#include "StageInit_2.h"
#include "StageInit_3.h"
#include"StageInit_4.h"
#include "StageInit_choseStage.h"
#include"ItemManager.h"
void ChangeStage(Stage& stage, int stageNo,EnemyManager& enemyManager, ItemManager& itemManager) {
    StageClear(stage);
    switch (stageNo) {
    case 1: StageInit_1(stage, enemyManager,itemManager); break;
    case 2: StageInit_2(stage, enemyManager,itemManager); break;
    case 3: StageInit_3(stage, enemyManager,itemManager); break;
    case 4: StageInit_4(stage, enemyManager,itemManager); break;
    case 99: StageInit_debug(stage,enemyManager,itemManager); break;
    case 0: StageInit_choseStage(stage, enemyManager); break;
        // •K—v‚É‰ž‚¶‚Ä’Ç‰Á
    }
}