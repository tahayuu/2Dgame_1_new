#include "Stage.h"
#include"EnemyManager.h"
#include"ItemManager.h"

// StageInit_deback.cpp の役割: デバッグ検証用の集約ステージを作る。
// 注意: 試験用に多種ギミックを同居させているため、本番ステージ設定とは切り分ける。
void StageInit_debug(Stage& stage,EnemyManager& enemyManager,ItemManager& itemManager) {

	// デバッグ用テクスチャ（パスは必要に応じて変更）
	StageThemeLoadAll(stage.theme,
		"assets/images/stage/stage_1/ground1.png",
		"assets/images/stage/stage_1/ground2.png",
		"assets/images/stage/stage_1/ground3.png",
		"assets/images/stage/stage_1/ground4.png",
		50.0f);

	//落下文字
  //stage.fallingTextCount = 1;
	  // 氷床
  //stage.icePlatformCount = 1;
	  //移動低下床
  //stage.moveDownPlatformCount = 1;
  //移動上昇床
  //stage.moveUpPlatformCount = 1;
  // 回転鉄球の初期化
  //stage.rotatingBallCount = 1;
  // 動く鉄球の初期化
	  //stage.moveRotatingBallCount = 1;
  // 転がる鉄球の初期化
	  //stage.rollingBallCount = 1;
  //追尾するとげ
	  //stage.trackingHazardCount = 1;
	  //往復上昇床
  //吹っ飛ばし壁
	stage.knockBackWallCount = 1;
	//ジャンプ台
	stage.jumpPlatfromCount = 1;
	//人を発射する砲台
	stage.batteryHumanCount = 1;
	//円軌道床
	//stage.circlePlatformCount = 1;
	//stage.upDownCount = 1;
	stage.platforms[0] = { 0.0f, 650.0f, 2000.0f, 70.0f };
	stage.platforms[1] = { 0.0f, 50.0f, 2000.0f, 70.0f };
	//とげ
	stage.hazards[0] = { 150, 620, 90, 30 };
	//カーソル追従床
	//stage.cursorPlatformCount = 1;
	//磁石床（引き寄せ）
	//stage.magnetCount = 1;
	//壊せるブロック
	//stage.breakableBlockCount = 1;
	//アイテムブロック
	//stage.itemBlockCount = 1;
	// 他のオブジェクトは最小限に
	stage.platformCount = 4;
	//stage.hazardCount = 1;
	//stage.moveCount = 0;
	//stage.moveExtYCount = 0;
	//stage.moveExtXCount = 0;
	//stage.fallingCount = 0;
	//stage.upRisingCount = 0;
	//stage.clearsCount = 0;
	//stage.movePlatformCountX = 1;
	//stage.fallingTextCount = 1;
	stage.switchPlatformCount = 3;
	// 落下文字
	//stage.fallingTexts[0] = { false,false,false,0.0f,0.0f,300,48,{2300.0f, 200.0f ,800,70},{2300,200},1600.0f,6,400.0f,"SAFEeeeee" };
	// 重要: rect と pos を一致させる（描画と当たり判定のズレ防止）
	stage.fallingTexts[0].pos.x = stage.fallingTexts[0].rect.x;
	stage.fallingTexts[0].pos.y = stage.fallingTexts[0].rect.y;

	//スイッチによって動作する床
	stage.switchPlatforms[0] = {
		{1400.0f,600.0f,300.0f,50.0f}, // rect
		{1400.0f,400.0f,300.0f,200.0f},//SwitchRect
		{1300.0f,0.0f},//speed
		1400.0f, //startX
		50.0f, //MaX
		false
	}; 

	// 氷床
	stage.icePlatforms[0] = {
		{400.0f, 650.0f, 300.0f, 20.0f},  // rect: 位置とサイズ
		0.02f,                             // friction: 摩擦係数（小さいほど滑る）
		400.0f,                            // slideAccel: 滑り加速度
		500.0f                             // maxSlideSpeed: 最大滑り速度
	};

	//移動低下床
	stage.moveDownPlatforms[0] = { {800.0f,600.0f,300.0f,50.0f} };
	//移動上昇床
	stage.moveUpPlatforms[0] = { {1200.0f,600.0f,1500.0f,50.0f} };

	// 回転鉄球の初期化
	stage.rotatingBalls[0] = {
		{700.0f, 300.0f},  // center: 中心位置
		50.0f,             // radius: 鉄球の半径
		1000.0f,            // armLength: 腕の長さ
		0.0f,              // angle: 初期角度
		2.0f               // rotationSpeed: 回転速度（2ラジアン/秒 ≈ 1回転/3秒）
	};
	stage.rotatingBallsInit[0] = stage.rotatingBalls[0];
	stage.exitDoors[0] = { {500.0f, 450.0f, 60.0f, 100.0f}, 0};//0=ステージ選択ステージ
	stage.exitDoorCount = 1;
	// 動く鉄球の初期化w

		// 動く鉄球の初期化
	stage.moveRotatingBalls[0] = MoveRotatingBall();
	stage.moveRotatingBalls[0].center = { 400.0f, 600.0f };  // 初期位置（高い位置）
	stage.moveRotatingBalls[0].radius = 20.0f;
	stage.moveRotatingBalls[0].armLength = 80.0f;
	stage.moveRotatingBalls[0].angle = 300.0f;// 初期角度
	stage.moveRotatingBalls[0].angularSpeed = 3.0f;  // 回転速度
	stage.moveRotatingBalls[0].isFalling = true;     // 最初は落下
	stage.moveRotatingBalls[0].onGround = false;
	stage.moveRotatingBalls[0].gravity = 1600.0f;

	stage.moveRotatingBallsInit[0] = stage.moveRotatingBalls[0];

	// 転がる鉄球の初期化

	stage.rollingBalls[0] = RollingBall();
	stage.rollingBalls[0].center = { 300.0f, 300.0f };  // 初期位置
	stage.rollingBalls[0].radius = 25.0f;             // 鉄球の大きさ
	stage.rollingBalls[0].rollSpeed = 150.0f;         // 転がる速度
	stage.rollingBalls[0].rollDerection = 1.0f;       // 右方向
	stage.rollingBalls[0].gravity = 1600.0f;          // 重力
	stage.rollingBalls[0].onGround = false;           // 落下開始

	stage.rollingBallsInit[0] = stage.rollingBalls[0];

	//追尾するとげ

	stage.trackingHazards[0].startPos = { 300.0f,560.0f };
	stage.trackingHazards[0].speed = 100.0f;
	stage.trackingHazards[0].trackingRange = 400.0f;
	stage.trackingHazards[0].maxDistance = 500.0f;
	stage.trackingHazards[0].isTracking = false;
	stage.trackingHazards[0].returnToStart = false;
	stage.trackingHazards[0].rect = { 300.0f,560.0f,90.0f,30.0f };


	// 円軌道床
	stage.circlePlatforms[0] = {
		{100.0f,100.0f},// vel
		{ 1500.0f, 400.0f},// center
		50.0f,             // radius
		0.0f,              // angle
		1.5f,              // angleSpeed
		100.0f,             // armLength
		false              // onPlayer
	};
	//動く床
	stage.movePlatformsX[0] = { 200.0f,300.0f,0.0f,0.1f,false,false,false,{600.0f,600.0f,150.0f,20.0f} };

	//吹っ飛ばし壁
	stage.knockBackWalls[0].rect = { 800.0f, 550.0f, 50.0f, 100.0f };
	stage.knockBackWalls[0].startX = 800.0f;  // ← rect.x と同じ値に！
	stage.knockBackWalls[0].knockBackVelocity = { 900.0f, -700.0f };  // 右に押し出す、少し上に
	stage.knockBackWalls[0].withdraw = 50.0f;  // 50px引っ込む
	stage.knockBackWalls[0].delay = 0.1f;
	stage.knockBackWalls[0].limitStop = 0.3f;
	stage.knockBackWalls[0].onTouch = false;
	stage.knockBackWalls[0].twiceTouch = false;
	stage.knockBackWalls[0].iswithdraw = false;
	stage.knockBackWalls[0].isRetrun = false;
	stage.knockBackWalls[0].timer = 0.0f;
	stage.knockBackWalls[0].stoptimer = 0.0f;

	//ジャンプ台
	stage.jumpPlatfroms[0].rect = { 1200.0f,600.0f,100.0f,20.0f };
	stage.jumpPlatfroms[0].jumpVelocity = { 100.0f,-1000 };
	stage.jumpPlatfroms[0].startY = 600.0f;

	//人を発射する砲台
	stage.batteryHumans[0].rect = { 1600.0f, 500.0f, 50.0f, 50.0f };
	stage.batteryHumans[0].BatteryVelocity = { 0.0f, -900.0f };

	// ゆっくり追従する床
	stage.cursorPlatforms[0] = {
		{ 400.0f, 400.0f, 100.0f, 20.0f },  // rect: x, y, width, height
		150.0f,   // followSpeed: 遅め
		500.0f,   // maxDistance
		true      // isActive
	};

	// 磁石床（引き寄せ）
	stage.magnets[0] = {
		{ 600.0f, 400.0f, 40.0f, 50.0f },  // rect: x, y, width, height
		50.0f,   // attractSpeed: 引き寄せ速度
		500.0f,   // attractRange: 引き寄せ範囲
		false      // isActive
	};
	//壊れるブロック
	stage.breakableBlocks[0] = { {200.0f,500.0f,50.0f,50.0f},false };
	//アイテムブロック
	stage.itemBlocks[0] = { {300.0f,500.0f,50.0f,50.0f}, false, ItemType::speedPotion, false };
	// 保存用初期化データにコピー
	stage.itemBlocksInit[0] = stage.itemBlocks[0];

	// ボタンブロック（下から叩くとブロック出現）
	stage.buttonBlocks[0] = {
		{500.0f, 500.0f, 50.0f, 50.0f},{
			
			// ボタン本体（空中に配置）
		{700.0f, 400.0f, 100.0f, 50.0f},{850.0f, 400.0f, 100.0f, 50.0f},{1000.0f, 400.0f, 100.0f, 50.0f}
		},// 出現するブロック
		false
	};
	stage.buttonBlockCount = 1;
	
	// 重力反転ブロック
	stage.gravityBlocks[0] = { {800, 300, 40, 40} };
	stage.gravityBlocksInit[0] = stage.gravityBlocks[0];
	stage.gravityBlockCount = 1;
	//下から触れただけで壊れるブロック
	stage.bottomBreakBlocks[0] = { {400.0f, 500.0f, 50.0f, 50.0f}, false };
	stage.bottomBreakBlockCount = 1;
	//エレベーター
	stage.elevators[0] = {
	{ 1000.0f, 650.0f, 150.0f, 20.0f }, // rect（初期位置＝地下の底付近）
	200.0f,    // upperY（地上付近まで上昇）
	650.0f,   // lowerY（地下の底）
	300.0f,    // speed
	false,     // goingUp
	false,     // goingDown
	false      // onPlayer
	};
	stage.elevatorsInit[0] = stage.elevators[0];
	stage.elevatorCount = 1;


	// =====================================================
//  奥行きレイヤー（手前/奥の切り替え）
// =====================================================

// 奥側の床（手前とは別の構造）
	stage.backPlatforms[0] = { -100.0f, 500.0f, 2000.0f, 70.0f };   // 奥の地面
	stage.backPlatforms[1] = { 0.0f, 50.0f, 2000.0f, 70.0f };    // 奥の天井
	stage.backPlatforms[2] = { 400.0f, 450.0f, 200.0f, 20.0f };  // 奥にだけある足場
	stage.backPlatforms[3] = { 700.0f, 350.0f, 200.0f, 20.0f };  // 奥にだけある足場
	stage.backPlatformCount = 4;

	// 奥行きドア（手前↔奥をつなぐ）
	stage.layerDoors[0] = {
		{ 200.0f, 550.0f, 60.0f, 100.0f },  // frontRect（手前のドア）
		{ 200.0f, 400.0f, 60.0f, 100.0f }   // backRect （奥のドア、同じ位置）
	};
	stage.layerDoors[1] = {
		{ 900.0f, 550.0f, 60.0f, 100.0f },  // frontRect（手前のドア）
		{ 900.0f, 250.0f, 60.0f, 100.0f }   // backRect （奥のドア、高い位置）
	};
	stage.layerDoorCount = 2;


	stage.touchBreakBlocks[0] = { {300.0f, 600.0f, 80.0f, 50.0f}, false, false, 0.0f, 0.5f };
	stage.touchBreakBlocks[1] = { {390.0f, 600.0f, 80.0f, 50.0f}, false, false, 0.0f, 0.7f };
	stage.touchBreakBlocks[2] = { {480.0f, 600.0f, 80.0f, 50.0f}, false, false, 0.0f, 0.9f };
	stage.touchBreakBlockCount = 3;

	// 復活用
	for (int i = 0; i < stage.buttonBlockCount; i++) {
		stage.buttonBlocksInit[i] = stage.buttonBlocks[i];
	}

	for (int i = 0; i < stage.elevatorCount; i++)
		stage.elevatorsInit[i] = stage.elevators[i];


	// 初期化保存（リセット用）
	   //復活用	
	for (int i = 0; i < stage.moveExtYCount; i++) {
		stage.moveHazardsExtYInit[i] = stage.moveHazardsExtY[i];
	}
	for (int i = 0; i < stage.jumpPlatfromCount; i++) {
		stage.jumpPlatfromsInit[i] = stage.jumpPlatfroms[i];
	}
	for (int i = 0; i < stage.knockBackWallCount; i++) {
		stage.knockBackWallsInit[i] = stage.knockBackWalls[i];
	}
	for (int i = 0; i < stage.fallingTextCount; i++) {
		stage.fallingTextsInit[i] = stage.fallingTexts[i];
	}
	for (int i = 0; i < stage.moveCount; i++) {
		stage.moveHazardsInit[i] = stage.moveHazards[i];
	}
	for (int i = 0; i < stage.trackingHazardCount; i++) {
		stage.trackingHazardsInit[i] = stage.trackingHazards[i];
	}
	for (int i = 0; i < stage.rotatingBallCount; i++) {
		stage.rotatingBallsInit[i] = stage.rotatingBalls[i];
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
	for (int i = 0; i < stage.upDownCount; i++) {
		stage.upDouwnPlatformsInit[i] = stage.upDouwnPlatforms[i];
	}
	for (int i = 0; i < stage.breakableBlockCount; i++){
		stage.breakableBlocksInit[i] = stage.breakableBlocks[i];
	} 
	for (int i = 0; i < stage.clearsCount; i++) {
		stage.clearBlocks[i].clearflag = false;
	}
	for (int i = 0; i < stage.touchBreakBlockCount; i++) {
		stage.touchBreakBlocksInit[i] = stage.touchBreakBlocks[i];
	}
	for (int i = 0; i < stage.bottomBreakBlockCount; i++) {
		stage.bottomBreakBlocksInit[i] = stage.bottomBreakBlocks[i];
	}

	static const Rectangle SPLIT_BASES[] = {
	{1000.0f,550.0f,200.0f,50.0f},
	};
	//カウント設定

	int count = (int)(sizeof(SPLIT_BASES) / sizeof(SPLIT_BASES[0]));
	if (count > MAX_SPLITPLATFORM)count = MAX_SPLITPLATFORM;
	stage.splitPlatformCount = count;

}
