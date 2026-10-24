#include "Stage.h"
#include"EnemyManager.h"
#include"ItemManager.h"

void StageInit_debug(Stage& stage,EnemyManager& enemyManager,ItemManager& itemManager) {

	StageThemeLoadAll(stage.theme,
		"assets/images/stage/stage_1/ground1.png",
		"assets/images/stage/stage_1/ground2.png",
		"assets/images/stage/stage_1/ground3.png",
		"assets/images/stage/stage_1/ground4.png",
		50.0f);

	stage.knockBackWallCount = 1;

	stage.jumpPlatfromCount = 1;

	stage.batteryHumanCount = 1;

	stage.platforms[0] = { 0.0f, 650.0f, 2000.0f, 70.0f };
	stage.platforms[1] = { 0.0f, 50.0f, 2000.0f, 70.0f };


	static const Rectangle SPLIT_BASES[] = {
	{1000.0f,550.0f,200.0f,50.0f},
	};
	//カウント設定

	int count = (int)(sizeof(SPLIT_BASES) / sizeof(SPLIT_BASES[0]));
	if (count > MAX_SPLITPLATFORM)count = MAX_SPLITPLATFORM;
	stage.splitPlatformCount = count;

}
