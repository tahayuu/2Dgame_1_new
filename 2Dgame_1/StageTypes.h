#pragma once
#include"raylib.h"
#include"GameObjects.h"
#include "CameraController.h"
#include "StageTheme.h"
#include "TiledMap.h"  
#include "SpriteDatabase.h" 

// ================================================================
// StageTypes.h の役割
// ---------------------------------------------------------------
// ・Stage が保持する全ギミック配列と、その初期化用配列を定義する。
// ・ゲーム中の実行時データと、リセット用データを同居させる設計。
// ================================================================

static constexpr int MAX_PLATFORMS = 128;//普通床の最大数conteexpr:コンパイル時定数
static constexpr int MAX_HAZARDS = 16;  //とげの最大数
static constexpr int MAX_CLEAR = 16;    //透明ブロックの最大数
static constexpr int MAX_FALLING = 16; //落下床の最大数
static constexpr int MAX_MOVEHAZARD = 16; //動くとげの最大数
static constexpr int MAX_MOVEHAZARDEXTY = 16; //動くとげY拡張の最大数
static constexpr int MAX_MOVEHAZARDEXTX = 16; //動くとげX拡張の最大数
static constexpr int MAX_SPLITPLATFORM = 16; //わかれる床の最大数
static constexpr int MAX_UPRISING = 16; //上昇床の最大数
static constexpr int MAX_FALLINGTEXT = 16; //落下文字の最大数
static constexpr int MAX_MOVEPLATFORM = 16; //乗ると動く床の最大数

// ステージ実行時の中核データ。
// 注意: カウント値と配列の整合が崩れると描画/衝突の両方に影響する。
struct Stage {

    //普通床
    Rectangle platforms[MAX_PLATFORMS];
    int platformCount;

	//奥側の床
	Rectangle backPlatforms[MAX_PLATFORMS];
	int backPlatformCount;

	//奥行きレイヤー
	int currentLayer = 0;  // 0=手前, 1=奥
	static constexpr int MAX_LAYERDOORS = 8;
	LayerDoor layerDoors[MAX_LAYERDOORS];
	int layerDoorCount;
	static constexpr float BACK_LAYER_SCALE = 0.7f; // 奥レイヤーの見た目スケール

	//ステージテーマ（床テクスチャなど）
	StageTheme theme;

	//触れると壊れるブロック
	TouchBreakBlock touchBreakBlocks[MAX_PLATFORMS];
	TouchBreakBlock touchBreakBlocksInit[MAX_PLATFORMS];
	int touchBreakBlockCount;

	//下から触れただけで壊れるブロック
	BottomBreakBlock bottomBreakBlocks[MAX_HAZARDS];
	BottomBreakBlock bottomBreakBlocksInit[MAX_HAZARDS];
	int bottomBreakBlockCount = 0;

	//エレベーター
	static constexpr int MAX_ELEVATORS = 8;
	Elevator elevators[MAX_ELEVATORS];
	Elevator elevatorsInit[MAX_ELEVATORS];
	int elevatorCount;
	bool playerInElevator = false; //プレイヤーがエレベーター内か

	//重力反転ブロック
	GravityBlock gravityBlocks[MAX_PLATFORMS];
	GravityBlock gravityBlocksInit[MAX_PLATFORMS];
	int gravityBlockCount;
	bool gravityReversed = false; //重力反転中か
	float gravityCooldown = 0.0f; //重力反転の再発動クールダウン

	// ボタンブロック
	ButtonBlock buttonBlocks[MAX_PLATFORMS];
	ButtonBlock buttonBlocksInit[MAX_PLATFORMS];
	int buttonBlockCount;

	//アイテムブロック
	ItemBlock itemBlocks[MAX_PLATFORMS];
	ItemBlock itemBlocksInit[MAX_PLATFORMS];
	int itemBlockCount;

	//プレイヤーが持ち運べるばね
	static constexpr int MAX_SPRINGS = 8;
	Spring springs[MAX_SPRINGS];
	Spring springsInit[MAX_SPRINGS];
	int springCount;
	int heldSpringIndex = -1;

	//氷床
	IcePlatform icePlatforms[MAX_PLATFORMS];
	IcePlatform iceplatformsInit[MAX_PLATFORMS];
	int icePlatformCount;

	//移動低下床
	MoveDownPlatform moveDownPlatforms[MAX_PLATFORMS];
	MoveDownPlatform moveDownplatformsInit[MAX_PLATFORMS];
	int moveDownPlatformCount;

	//ジャンプ台
	JumpPlatfrom jumpPlatfroms[MAX_PLATFORMS];
	JumpPlatfrom jumpPlatfromsInit[MAX_PLATFORMS];
	int jumpPlatfromCount;
	bool playerJumped = false; // ジャンプ台で跳ねたか
	
	//人を発射する砲台
	BatteryHuman batteryHumans[MAX_PLATFORMS];
	BatteryHuman batteryHumansInit[MAX_PLATFORMS];
	int batteryHumanCount;
	bool playerFired = false; // 発射されたか
	bool playerInBattery = false; // 砲台内にいるか

	//カーソル追従床
	CursorPlatform cursorPlatforms[MAX_PLATFORMS];
	CursorPlatform cursorplatformsInit[MAX_PLATFORMS];
	int cursorPlatformCount;

	//カーソルによって動作するギミック
	CursorBottom cursorBottoms[MAX_PLATFORMS];
	CursorBottom cursorBottomsInit[MAX_PLATFORMS];
	int cursorBottomCount;

	//磁石（引き寄せ）
	Magnet magnets[MAX_PLATFORMS];
	Magnet magnetsInit[MAX_PLATFORMS];
	int magnetCount;

	//移動上昇床
	MoveUpPlatform moveUpPlatforms[MAX_PLATFORMS];
	MoveUpPlatform moveUpplatformsInit[MAX_PLATFORMS];

	//吹っ飛ばし壁
	KnockBackWall knockBackWalls[MAX_PLATFORMS];
	KnockBackWall knockBackWallsInit[MAX_PLATFORMS];
	int knockBackWallCount;
	bool playerKnockedBack = false;  // 吹っ飛ばされたか

	int moveUpPlatformCount;
    //わかれる床
    SplitPlatform splitPlatforms[MAX_SPLITPLATFORM];
    int splitPlatformCount;
    SplitPlatform splitplatformsInit[MAX_SPLITPLATFORM];//初期化のために保存しとく

    //円軌道床
	CirclePlatform circlePlatforms[MAX_PLATFORMS];
	CirclePlatform circleplatformsInit[MAX_PLATFORMS];	
	int circlePlatformCount;

	MovePlatformYXY movePlatformsYXY[MAX_MOVEPLATFORM];
	MovePlatformYXY movePlatformsYXYInit[MAX_MOVEPLATFORM];
	int movePlatformCountYXY;

	//乗ると動く床
	MovePlatformX movePlatformsX[MAX_MOVEPLATFORM];
	MovePlatformX moveplatformsXInit[MAX_MOVEPLATFORM];
	int movePlatformCountX;

	
    //とげ
    Rectangle hazards[MAX_HAZARDS];
	int hazardDisableSnapGroupIds[MAX_HAZARDS] = {};//無効化するスナップグループID
	int hazardDisableSignalIds[MAX_HAZARDS] = {};//無効化するシグナルID
	/*disableSnapGroupId
    → スナップパズル完成で無効化

    disableSignalId
    → 距離トリガーなどの信号で無効化*/
	int hazardCount;
	int moveCount;
	int moveExtYCount;

    //動くとげ
    MoveHazard moveHazards[MAX_MOVEHAZARD];
    MoveHazard moveHazardsInit[MAX_MOVEHAZARD];//初期化のために保存しとく
	//上に上がるとげ
	MoveHazardExtentionY moveHazardsExtY[MAX_MOVEHAZARD];
	MoveHazardExtentionY moveHazardsExtYInit[MAX_MOVEHAZARD];
	//下に下がるとげ
	MoveDownHazardExtentionY moveDownHazardsExtY[MAX_MOVEHAZARD];
	MoveDownHazardExtentionY moveDownHazardsExtYInit[MAX_MOVEHAZARD];
	int moveDownHazardExtYCount;
	//左右どちらかに出るとげ
	MoveHazardRightX moveHazardsRight[MAX_MOVEHAZARD];
	MoveHazardRightX moveHazardsRightInit[MAX_MOVEHAZARD];
	int moveHazardRightCount;
    //追尾するとげ 
	TrackingHazard trackingHazards[MAX_HAZARDS];
	TrackingHazard trackingHazardsInit[MAX_HAZARDS];
	int trackingHazardCount;
	//スパイク型トランポリン
	static constexpr int MAX_SPIKE_BOUNCERS = 16;
	SpikeBouncer spikeBouncers[MAX_SPIKE_BOUNCERS];
	SpikeBouncer spikeBouncerInit[MAX_SPIKE_BOUNCERS];
	int spikeBouncerCount = 0;
	//死ぬブロック
	Rectangle deathBlocks[MAX_HAZARDS];
	Rectangle deathBlocksInit[MAX_HAZARDS];
	int deathBlockCount;

    //回転する鉄球
	RotatingBall rotatingBalls[MAX_HAZARDS];
	RotatingBall rotatingBallsInit[MAX_HAZARDS];
	int rotatingBallCount;

	//ワイヤーあり回転する鉄球
    MoveRotatingBall moveRotatingBalls[MAX_HAZARDS];
    MoveRotatingBall moveRotatingBallsInit[MAX_HAZARDS];
    int moveRotatingBallCount;
	//転がる鉄球
	RollingBall rollingBalls[MAX_HAZARDS];
	RollingBall rollingBallsInit[MAX_HAZARDS];
    int rollingBallCount;

    //落下床
    FallingPlatform fallingPlatforms[MAX_FALLING];
    FallingPlatform fallingPlatformsInit[MAX_FALLING];
    int fallingCount;
    float stageWidth;

	//上昇床
	UpRisingPlatform upRisingPlatforms[MAX_UPRISING];
	UpRisingPlatform upRisingPlatformsInit[MAX_UPRISING];
	int upRisingCount;
	float stageHeight;
	bool playerOnUpRising = false; 
	
	//往復上昇床
	UpDownPlatform upDouwnPlatforms[MAX_UPRISING];
	UpDownPlatform upDouwnPlatformsInit[MAX_UPRISING];
	int upDownCount;

    //透明ブロック
	ClearBlock clearBlocks[MAX_CLEAR];
	ClearBlock clearBlocksInit[MAX_CLEAR];
    int clearsCount;

	//X方向透明ブロック
	ClearBlockX clearBlocksX[MAX_CLEAR];
	ClearBlockX clearBlocksXInit[MAX_CLEAR];
	int clearsXCount;

	//スイッチによって動く床
	SwitchPlatform switchPlatforms[MAX_PLATFORMS];
	SwitchPlatform switchplatformsInit[MAX_PLATFORMS];
	int switchPlatformCount;


    //落下文字
	FallingText fallingTexts[MAX_FALLINGTEXT];
	FallingText fallingTextsInit[MAX_FALLINGTEXT];
    int fallingTextCount;
    
	//壊せるブロック
	BreakableBlock breakableBlocks[MAX_HAZARDS];
	BreakableBlock breakableBlocksInit[MAX_HAZARDS];
	int breakableBlockCount;

	//出口ドア
	static constexpr int MAX_EXITDOORS = 10;
	ExitDoor exitDoors[MAX_EXITDOORS];
	int exitDoorCount;
	int exitDoorTriggered = -1; // 接触したexitDoorのインデックス（-1=接触なし）

	//ワープホール
	static constexpr int MAX_WARPS = 10;
	Warp warps[MAX_WARPS];
	int warpCount;
	int warpTriggered = -1; // 接触したwarpHoleのインデックス（-1=接触なし）

	// リスポーン地点（エディタで配置）
	Vector2 respawnPoint = { 100.0f, 500.0f };
	bool hasRespawnPoint = false;
	//コメントブロック
	static constexpr int MAX_COMMENT_BLOCKS = 16;
	CommentBlock commentBlocks[MAX_COMMENT_BLOCKS];
	CommentBlock commentBlocksInit[MAX_COMMENT_BLOCKS];
	int commentBlockCount = 0;
	CameraConfig cameraConfig;

	// クレーン発射台
	static constexpr int MAX_CRANES = 8;
	CraneLaunchPad craneLaunchPads[MAX_CRANES];
	CraneLaunchPad craneLaunchPadsInit[MAX_CRANES];
	int craneLaunchPadCount = 0;

	// クレーン（天井ギミック）
	Crane cranes[MAX_CRANES];
	Crane cranesInit[MAX_CRANES];
	int craneCount = 0;
	bool playerGrabbedByCrane = false; // クレーンに掴まれているか
	bool playerCraneKill = false;      // クレーンによる死亡フラグ
	bool playerLaunched = false;

	// おじさんのパンチトリガーエリア
	static constexpr int MAX_OJISAN_PUNCH_AREAS = 4;
	Rectangle ojisanPunchAreas[MAX_OJISAN_PUNCH_AREAS];
	bool ojisanPunchTriggered[MAX_OJISAN_PUNCH_AREAS];
	// false: 拳をステージオブジェクトの後ろ / true: 前に描画する
	bool ojisanPunchDrawFront[MAX_OJISAN_PUNCH_AREAS] = {};
	// 拳が接近を開始してからプレイヤーが死亡するまでの時間（秒）
	float ojisanPunchApproachDuration[MAX_OJISAN_PUNCH_AREAS] = {};
	int ojisanPunchAreaCount = 0;
	
	// 一時的に消える床
	static constexpr int MAX_TEMP_FLOORS = 16;
	TempFloor tempFloors[MAX_TEMP_FLOORS];
	TempFloor tempFloorsInit[MAX_TEMP_FLOORS];
	int tempFloorCount = 0;

	// 一時的に消える床のスイッチ
	TempFloorSwitch tempFloorSwitches[MAX_TEMP_FLOORS];
	TempFloorSwitch tempFloorSwitchesInit[MAX_TEMP_FLOORS];
	int tempFloorSwitchCount = 0;

	static constexpr int MAX_DECOR_ARROWS = 16;
	DecorArrow decorArrows[MAX_DECOR_ARROWS];
	int decorArrowCount = 0;

	static constexpr int MAX_DRAG_PIECES = 32;
	static constexpr int MAX_SNAP_SLOTS = 32;

	DragPiece dragPieces[MAX_DRAG_PIECES];
	DragPiece dragPiecesInit[MAX_DRAG_PIECES];
	int dragPieceCount = 0;

	static constexpr int MAX_DISTANCE_TRIGGER_PIECES = 32;

	DistanceTriggerPiece distanceTriggerPieces[MAX_DISTANCE_TRIGGER_PIECES];
	DistanceTriggerPiece distanceTriggerPiecesInit[MAX_DISTANCE_TRIGGER_PIECES];

	int distanceTriggerPieceCount = 0;// 離れた距離で発動するピースの数
	int draggingDistanceTriggerPieceIndex = -1;// 離れた距離で発動するピースのドラッグ中インデックス
	bool distanceTriggerMouseCaptured = false;// 離れた距離で発動するピースのマウスキャプチャ中か
	static constexpr int MAX_GIMMICK_SIGNALS = 254;// ギミック信号の最大数
	bool gimmickSignals[MAX_GIMMICK_SIGNALS] = {};

	SnapSlot snapSlots[MAX_SNAP_SLOTS];// スナップスロットの配列
	SnapSlot snapSlotsInit[MAX_SNAP_SLOTS];// スナップスロットの初期化用配列
	int snapSlotCount = 0;

	int draggingSnapPieceIndex = -1;
	bool snapMouseCaptured = false;
	// ================================================================
// 見た目専用スプライト情報（当たり判定・ギミック処理には一切使わない）
// ----------------------------------------------------------------
// エディタで配置した PlacedObject の spriteId / rotation / flipX / flipY を
// そのまま保持するための「描画専用」データ。
// ここに入っている情報は EditorExportToStage で書き込まれ、
// StageDraw 側で見た目を上書きしたい場合にだけ参照される想定。
// （当たり判定は各ギミック配列(platforms等)の rect が引き続き使われる）
// ================================================================
	struct SpriteInstance {
		Rectangle rect = {};                 // 描画位置・大きさ（対応するギミックのrectと同じ値）
		SpriteId  spriteId = SpriteId::None;  // 描画に使う画像パーツ
		float     rotation = 0.0f;            // 見た目の回転角度（度）
		bool      flipX = false;              // 左右反転
		bool      flipY = false;              // 上下反転
	};
	static constexpr int MAX_SPRITE_INSTANCES = 512;
	SpriteInstance spriteInstances[MAX_SPRITE_INSTANCES];
	int spriteInstanceCount = 0;
	static constexpr int MAX_DECO_SPRITES = 128;
	DecoSprite decoSprites[MAX_DECO_SPRITES];
	DecoSprite decoSpritesInit[MAX_DECO_SPRITES];
	int decoSpriteCount = 0;

	static constexpr int MAX_EVENT_CHANGERS = 16;
	EventChanger eventChangers[MAX_EVENT_CHANGERS];
	int eventChangerCount = 0;

	// 0：SPACE/W/UP
	// 1：Nのみ
	int currentJumpMode = 0;



};
	