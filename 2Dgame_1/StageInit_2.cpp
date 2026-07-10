#include "Stage.h"

// StageInit_2.cpp の役割: ステージ2の固定初期配置を定義する。
// 注意: dialogKey 付き敵配置があり、DialogManager側のキー名と整合が必要。
void StageInit_2(Stage& stage, EnemyManager& enemyManager, ItemManager& itemManager) {//&は参照（本物を使う）よってmain.cppに反映される

	StageThemeLoadAll(stage.theme,
		"assets/images/stage/stage_1/ground1.png",   // top (grass)
		"assets/images/stage/stage_1/ground2.png",   // body (dirt)
		"assets/images/stage/stage_1/ground3.png",   // left edge
		"assets/images/stage/stage_1/ground4.png",   // right edge
		50.0f);
	StageThemeLoad(stage.theme,
		"assets/images/stage/stage_1/ground1.png",
		"assets/images/stage/stage_1/ground2.png",
		50.0f);
	/*StageThemeLoadObjectTextures(
		stage.theme,
		"assets/images/stage/stage_1/itemblock.png",
		"assets/images/stage/stage_1/normalblock.png",
		"assets/images/items/Arrow.png",
		"assets/images/stage/stage_1/Bullet.png"
	);*/
	//普通床
	stage.platforms[0] = { 0, 600, 2500, 200 };
	stage.platforms[1] = { 1000, 500, 100, 100 };
	stage.platforms[2] = { 1100, 400, 300, 400 };
	stage.platforms[3] = { 1400, 500, 200, 100 };
	stage.platforms[4] = { 2400, 400, 100, 600 };
	stage.platforms[5] = { 2800, 400, 100, 600 };
	stage.platforms[7] = { 4450, 500, 200, 70 };
	stage.platforms[8] = { 5400, 550, 800, 400 };
	stage.platforms[9] = { 2900, 600, 100, 200 };
	//stage.platforms[6] = { 3800, 600, 500, 50 };
	//stage.platforms[10] = { 3200, 600, 150, 200 };

	//アイテムブロック
	stage.itemBlocks[0] = { {1250.0f,200.0f,50.0f,50.0f}, false, ItemType::speedPotion, false };
	stage.itemBlocks[1] = { {2750.0f,400.0f,60.0f,60.0f},false };
	stage.itemBlocks[2] = { {4550.0f,330.0f,50.0f,50.0f},false,ItemType::excessJumpPotion };
	//壊せるブロック
	stage.breakableBlocks[0] = { {1200.0f,200.0f,50.0f,50.0f},false };
	stage.breakableBlocks[1] = { {1300.0f,200.0f,50.0f,50.0f},false };

	//敵キャラ配置
	enemyManager.Spawn(EnemyType::WALKER, { 700.0f, 550.0f });
	enemyManager.enemies.back().dialogKey = "death_enemy_walker_patrol";  // 巡回中の敵

	enemyManager.Spawn(EnemyType::FLYER, { 1820.0f, 400.0f });
	enemyManager.enemies.back().dialogKey = "death_enemy_flyer";          // 飛んでいる敵

	enemyManager.Spawn(EnemyType::WALKER, { 1900.0f, 550.0f });
	enemyManager.enemies.back().dialogKey = "death_enemy_walker_stage2";  // ステージ2の敵

	enemyManager.Spawn(EnemyType::WALKER, { 2600.0f, 550.0f });
	//透明ブロック（横）
	stage.clearBlocksX[0] = { 4050,450,50,50 };
	stage.clearBlocksX[1] = { 4050,500,50,50 };
	stage.clearBlocksX[2] = { 4050,550,50,50 };
	stage.clearBlocksX[3] = { 4050,400,50,50 };
	//stage.clearBlocksX[4] = { 4050,350,50,50 };

	// 落下文字
	stage.fallingTexts[0] = { false,false,false,0.0f,0.0f,300,48,{5800.0f, 200.0f ,130,60},
		{2300,200},1600.0f,6,400.0f,"NEXT",{5800,200} };

	//透明ブロック
	stage.clearBlocks[0] = { 600, 410,60,60 };
	stage.clearBlocks[1] = { 2500, 400,60,60 };
	stage.clearBlocks[2] = { 2560, 400,60,60 };
	stage.clearBlocks[3] = { 2620, 400,60,60 };
	stage.clearBlocks[4] = { 2680, 400,80,60 };
	stage.clearBlocks[5] = { 2530, 200,50,50 };
	stage.clearBlocks[10] = { 3600,150,100,50 };
	stage.clearBlocks[6] = { 3700,150,100,50 };
	stage.clearBlocks[7] = { 3800,150,50,50 };
	stage.clearBlocks[8] = { 3850,50,200,20 };
	stage.clearBlocks[9] = { 4050,150,300,50 };
	//動くトゲ
	stage.moveHazardsExtY[0] = { { 1500,470,90,30 },false,false,400.0f,470.0f,1000.0f,-1 };
	//下に下がるとげw
	stage.moveDownHazardsExtY[0] = { { 1400,-60,90,30 },false,false,300.0f,-60.0f,1500.0f,1,100.0f };
	stage.moveDownHazardsExtY[1] = { { 3200,-30,400,30 },false,false,500.0f,-60.0f,1500.0f,1,200.0f };

	//出てくるトゲ
	stage.moveHazards[0] = { { 1420,520,70,50 },false,false,50.0f,520.0f,200.0f };
	//動くとげ右
	//stage.moveHazardsExtX[0] = { { 1310,470,90,30 }, false, false, 300.0f, 1900.0f, 300.0f,0.0f,1.0f };
	//動くとげ右（Y軸厳密）
	stage.moveHazardsRight[0] = { { 1310,470,90,30 }, false, false, 300.0f, 1900.0f, 800.0f,0.0f,1.0f };
	//上昇床
	stage.upRisingPlatforms[0] = { {0,599,500,200},false,false,2.0f,0.0f,1500.0f };
	stage.upRisingPlatforms[1] = { {3200,599,150,200},false,false,2.0f,0.0f,1300.0f };
	//落下床                            
	stage.fallingPlatforms[0] = { {2500,580,300,400},false,false,5.0f,0.0f,450.0f };
	//stage.fallingPlatforms[1] = { {700,500,150,50},false,false,0.2f,0.0f,450.0f };
	//吹っ飛ばし壁
	stage.knockBackWalls[0] = { { 2380,400,50,200 },2380.0f, { 900.0f, -700.0f },false,false ,false,false,0.0f,
								0.1f,50.0f,0.0f,0.3f };
	stage.knockBackWalls[1] = { { 2850,500,50,200 }, 2850.0f, { 200.0f, -300.0f }, false, false, false, false, 0.0f,
							   0.1f, -20.0f, 0.0f, 0.3f };
	//人を発射する砲台
	stage.batteryHumans[0].rect = { 3650.0f, 350.0f, 50.0f, 50.0f };
	stage.batteryHumans[0].BatteryVelocity = { -300.0f, -800.0f };
	stage.batteryHumans[0].delay = 4.0f;
	stage.batteryHumans[1].rect = { 4200.0f, 50.0f, 50.0f, 50.0f };
	stage.batteryHumans[1].BatteryVelocity = { 1500.0f, -800.0f };
	stage.batteryHumans[1].delay = 3.0f;

	//ジャンプ台
	stage.jumpPlatfroms[0].rect = { 3600.0f,600.0f,200.0f,200.0f };
	stage.jumpPlatfroms[0].jumpVelocity = { 0.0f,-1500 };
	stage.jumpPlatfroms[0].startY = 600.0f;
	stage.jumpPlatfroms[0].delay = 4.0f;

	//スイッチによって動作する床
	//stage.switchPlatforms[0] = { {3600.0f,700.0f,200.0f,20.0f},{4020,350,300,300 },{1000,0 },200,3600,false };
	//stage.switchPlatforms[1] = { {5200.0f,550.0f,1200.0f,400.0f},{5200,350,500,300 },{1000,0 },100,5200,false };
	//わかれる床
	static const Rectangle SPLIT_BASES[] = {
		{3000.0f,600.0f,200.0f,200.0f},
	};

	stage.exitDoors[0] = { {6100.0f, 450.0f, 60.0f, 100.0f}, 0};
	stage.exitDoorCount = 1;
	


	//カウント設定
	stage.stageWidth = 6400.0f;      // ← 追加：ゴール判定の右端
	stage.clearsXCount = 6;
	stage.batteryHumanCount = 3;
	stage.knockBackWallCount = 6;
	stage.fallingCount = 3;
	stage.upRisingCount = 5;
	stage.moveCount = 1;
	stage.moveExtYCount = 1;
	stage.platformCount = 16;
	stage.hazardCount = 4;
	stage.movePlatformCountX = 1;
	stage.upDownCount = 1;
	stage.itemBlockCount = 4;
	stage.breakableBlockCount = 2;
	stage.clearsCount = 10;
	stage.moveDownHazardExtYCount = 2;
	stage.moveHazardRightCount = 1;
	stage.splitPlatformCount = 1;
	stage.jumpPlatfromCount = 2;
	stage.switchPlatformCount = 2;  // 3→2（初期化されているのは[0][1]だけ）
	stage.fallingTextCount = 1;

	for (int i = 0; i < stage.splitPlatformCount; i++) {
		auto& sp = stage.splitPlatforms[i];
		sp.base = SPLIT_BASES[i];
		sp.triggered = false;
		sp.offsetX = 0.0f;
		sp.maxGap = 60.0f;
		sp.splitSpeed = 450.f;
		sp.left = { sp.base.x,sp.base.y,sp.base.width / 2.0f,sp.base.height };
		sp.right = { sp.base.x + sp.base.width / 2.0f,sp.base.y,sp.base.width / 2.0f ,sp.base.height };
		sp.baseInit = SPLIT_BASES[i];
	}


	//復活用	
	for (int i = 0; i < stage.moveExtYCount; i++) {
		stage.moveHazardsExtYInit[i] = stage.moveHazardsExtY[i];
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
		stage.itemBlocksInit[i] = stage.itemBlocks[i];
	}
	for (int i = 0; i < stage.upDownCount; i++) {
		stage.upDouwnPlatformsInit[i] = stage.upDouwnPlatforms[i];
	}
	for (int i = 0; i < stage.moveDownHazardExtYCount; i++) {
		stage.moveDownHazardsExtYInit[i] = stage.moveDownHazardsExtY[i];
	}
	for(int i= 0; i < stage.moveHazardRightCount; i++) {
		stage.moveHazardsRightInit[i] = stage.moveHazardsRight[i];
	}	
	for (int i = 0; i < stage.clearsCount; i++) {
		stage.clearBlocksInit[i] = stage.clearBlocks[i];
	}
	for (int i = 0; i < stage.knockBackWallCount; i++) {
		stage.knockBackWallsInit[i] = stage.knockBackWalls[i];
	}
	for (int i = 0; i < stage.jumpPlatfromCount; i++) {
		stage.jumpPlatfromsInit[i] = stage.jumpPlatfroms[i];
	}
	for (int i = 0; i < stage.clearsXCount; i++) {
		stage.clearBlocksXInit[i] = stage.clearBlocksX[i];
	}
	for (int i = 0; i < stage.switchPlatformCount; i++) {
		stage.switchplatformsInit[i] = stage.switchPlatforms[i];
	}
	for(int i = 0;i<stage.fallingTextCount;i++){
		stage.fallingTextsInit[i] = stage.fallingTexts[i];
	}
}

