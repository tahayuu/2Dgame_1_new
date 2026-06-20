#pragma once
#include "Stage.h"
#include "StageInit_deback.h"
#include "StageInit_1.h"
#include "StageInit_2.h"
#include "StageInit_3.h"
#include "StageInit_choseStage.h"

void ChangeStage(Stage& stage, int stageNo, EnemyManager& enemyManager, ItemManager& itemManager);