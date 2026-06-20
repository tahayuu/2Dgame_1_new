#include"Stage.h"
#include "StageInit_choseStage.h"

void  StageInit_4(Stage& stage, EnemyManager& enemyManager, ItemManager& itemManager)
{
	// --- ここから実際の配置 ---
	stage.stageWidth = 3000.0f;

	//普通床
	stage.platforms[0] = { 0, 600, 2000, 150 };
	stage.platforms[1] = { 400, 140, 200, 60 };
	stage.platforms[2] = { 1600, 300, 200, 60 };
	stage.platforms[3] = { 1000, 300, 200, 60 };
	stage.platformCount = 4;










	//分割床(配置表)
	static const Rectangle SPLIT_BASES[] = {
		{1000.0f, 550.0f, 200.0f, 50.0f},
	};
	stage.splitPlatformCount = 0;

	for (int i = 0; i < stage.splitPlatformCount; i++) {
		auto& sp = stage.splitPlatforms[i];
		sp.base = SPLIT_BASES[i];
		sp.triggered = false;
		sp.offsetX = 0.0f;
		sp.maxGap = 60.0f;
		sp.splitSpeed = 450.0f;
		sp.left = { sp.base.x, sp.base.y, sp.base.width / 2.0f, sp.base.height };
		sp.right = { sp.base.x + sp.base.width / 2.0f, sp.base.y, sp.base.width / 2.0f, sp.base.height };
		sp.baseInit = SPLIT_BASES[i];
		stage.splitplatformsInit[i] = sp;
	}

	// 復活用
	for (int i = 0; i < stage.platformCount; i++) {
		// 普通床はリセット不要（位置固定）
	}

	// 敵をリセット
	enemyManager.Init();
}