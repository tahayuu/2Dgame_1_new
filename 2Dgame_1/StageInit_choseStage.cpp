#include"Stage.h"
#include "StageInit_choseStage.h"

// StageInit_choseStage.cpp の役割: ステージ選択用マップを初期化する。
// 目的: 出口ドアで targetStage を選ばせるハブステージを作る。
void StageInit_choseStage(Stage& stage, EnemyManager& enemyManager)
{
	// ステージ選択画面用テクスチャ（パスは必要に応じて変更）
	StageThemeLoadAll(stage.theme,
		"assets/images/stage/stage_1/ground1.png",
		"assets/images/stage/stage_1/ground2.png",
		"assets/images/stage/stage_1/ground3.png",
		"assets/images/stage/stage_1/ground4.png",
		50.0f);

	stage.stageWidth = 3000.0f;

	//普通床
	stage.platforms[0] = { 0, 600, 2000, 150 };
	stage.platforms[1] = { 400, 140, 200, 60 };
	stage.platforms[2] = { 1600, 300, 200, 60 };
	stage.platforms[3] = { 1000, 300, 200, 60 };
	stage.platformCount = 4;

	stage.exitDoorCount = 6;
	stage.exitDoors[0] = { {450.0f, 500.0f, 60.0f, 100.0f}, 2 };
	stage.exitDoors[1] = { {450.0f, 40.0f, 60.0f, 100.0f}, 5  };
	//stage.exitDoors[] = { {450.0f, 40.0f, 60.0f, 100.0f}, 3 };
	stage.exitDoors[2] = { {1100.0f, 200.0f, 60.0f, 100.0f}, 3 };
	/*0	StageInit_choseStage（ステージ選択）
      1	StageInit_1（イージーモード）
      2	StageInit_2（ステージ１）
      3	StageInit_3(ステージ2)
	  4	StageInit_4（ステージ3）
      99	StageInit_debug（デバッグモード）
*/
// ゆっくり追従する床
	stage.cursorPlatforms[0] = {
		{ 730.0f, 400.0f, 100.0f, 20.0f },  // rect: x, y, width, height
		150.0f,   // followSpeed: 遅め
		500.0f,   // maxDistance
		true      // isActive
	};
	stage.cursorPlatformCount = 1;

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
		sp.left  = { sp.base.x, sp.base.y, sp.base.width / 2.0f, sp.base.height };
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