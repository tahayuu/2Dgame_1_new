#include "Stage.h"

// StageInit_1.cpp の役割: ステージ1の固定初期配置を定義する。
// 目的: 地形/敵/アイテム/初期化用配列を設定する。
void StageInit_1(Stage& stage, EnemyManager& enemyManager, ItemManager& itemManager) {//&は参照（本物を使う）よってmain.cppに反映される


	StageThemeLoadAll(stage.theme,
		"assets/images/stage/stage_1/ground1.png",   // top (grass)
		"assets/images/stage/stage_1/ground2.png",   // body (dirt)
		"assets/images/stage/stage_1/ground3.png",   // left edge
		"assets/images/stage/stage_1/ground4.png",   // right edge
		50.0f);

	StageThemeLoadObjectTextures(
		stage.theme,
		"assets/images/stage/stage_1/itemblock.png",
		"assets/images/stage/stage_1/normalblock.png",
		"assets/images/items/Arrow.png"
	);
	//普通床
	stage.platforms[0] = { 0, 600, 3300, 200 };
	stage.platforms[1] = { 1000, 500, 100, 100 };
	stage.platforms[2] = { 1100, 400, 300, 400 };
	stage.platforms[3] = { 1400, 500, 200, 100 };
	stage.platforms[4] = { 2400, 400, 100, 600 };
	stage.platforms[5] = { 2800, 400, 100, 600 };
	stage.platforms[6] = { 3600, 600, 200, 600 };
	stage.platforms[7] = { 4450, 500, 200, 70 };
	stage.platforms[8] = { 5200, 550, 1200, 400 };

	//アイテムブロック
	stage.itemBlocks[0] = { {1250.0f,200.0f,50.0f,50.0f}, false, ItemType::speedPotion, false };
	stage.itemBlocks[1] = { {2750.0f,400.0f,50.0f,50.0f},false };
	stage.itemBlocks[2] = { {4550.0f,330.0f,50.0f,50.0f},false };
	//壊せるブロック
	stage.breakableBlocks[0] = { {1200.0f,200.0f,50.0f,50.0f},false };
	stage.breakableBlocks[1] = { {1300.0f,200.0f,50.0f,50.0f},false };

	//敵キャラ配置
	enemyManager.Spawn(EnemyType::WALKER, { 700.0f, 550.0f });
	enemyManager.Spawn(EnemyType::FLYER, { 1820.0f,400.0f });
	enemyManager.Spawn(EnemyType::WALKER, { 1900.0f, 550.0f });
	enemyManager.Spawn(EnemyType::WALKER, { 2600.0f, 550.0f });

	//トゲ
	stage.hazards[0] = { 1500, 470, 90, 30 };
	stage.hazardCount = 4;
	stage.movePlatformCountX = 1;
	stage.upDownCount = 1;
	stage.itemBlockCount = 4;
	stage.breakableBlockCount = 2;
	//カウント設定
	stage.platformCount = 10;

	// おじさんパンチ演出トリガー
	stage.ojisanPunchAreaCount = 1;
	stage.ojisanPunchAreas[0] = { 2350.0f, 420.0f, 220.0f, 220.0f };
	stage.ojisanPunchTriggered[0] = false;

	//復活用	
	for (int i = 0; i < stage.moveExtYCount; i++) {
		stage.moveHazardsExtYInit[i] = stage.moveHazardsExtY[i];
	}
	for (int i = 0; i < stage.moveExtXCount; i++) {
		stage.moveHazardsExtXInit[i] = stage.moveHazardsExtX[i];
	}
	for (int i = 0; i < stage.fallingTextCount; i++) {
		stage.fallingTextsInit[i] = stage.fallingTexts[i];
	}
	for (int i = 0; i < stage.moveCount; i++) {
		stage.moveHazardsInit[i] = stage.moveHazards[i];
	}
	for (int i = 0; i < stage.fallingCount; i++) {
		stage.fallingPlatformsInit[i] = stage.fallingPlatforms[i];
	}
	for (int i = 0; i < stage.splitPlatformCount; i++) {
		stage.splitplatformsInit[i] = stage.splitPlatforms[i];
	}
	for (int i = 0; i < stage.upRisingCount; i++) {
		stage.upRisingPlatformsInit[i] = stage.upRisingPlatforms[i];
	}
	for (int i = 0; i < stage.moveCount; i++) {
		stage.moveplatformsXInit[i] = stage.movePlatformsX[i];
	}
	for (int i = 0; i < stage.breakableBlockCount; i++) {
		stage.breakableBlocksInit[i] = stage.breakableBlocks[i];
	}
	for (int i = 0; i < stage.itemBlockCount; i++) {
		stage.itemBlocksInit[i]=stage.itemBlocks[i];
	}
	for (int i = 0; i < stage.upDownCount; i++) {
		stage.upDouwnPlatformsInit[i] = stage.upDouwnPlatforms[i];
	}
}

