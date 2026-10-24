#pragma once
#include"raylib.h"
#include"GameObjects.h"
#include "CameraController.h"
#include "StageTheme.h"
#include "TiledMap.h"  
#include "SpriteDatabase.h" 
#include "GimmickGroup.h"

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

	// ================================================================
	// GimmickGroup<T,N> 移行済みギミック（実体・初期状態・登録数を一元管理）
	// ---------------------------------------------------------------
	// Add()で実体とinitialObjectsへ同時保存し、Get()/Size()/Reset()/Clear()を提供。
	// 見通しを良くするため、移行済みのものはここにまとめている。
	// ================================================================
	static constexpr int MAX_COMMENT_BLOCKS = 16;
	static constexpr int MAX_TEMP_FLOORS = 16;
	static constexpr int MAX_DECOR_ARROWS = 16;
	static constexpr int MAX_SPIKE_BOUNCERS = 16;

	//触れると壊れるブロック
	GimmickGroup<TouchBreakBlock, MAX_PLATFORMS> touchBreakBlocks;
	//下から触れただけで壊れるブロック
	GimmickGroup<BottomBreakBlock, MAX_HAZARDS> bottomBreakBlocks;
	//重力反転ブロック
	GimmickGroup<GravityBlock, MAX_PLATFORMS> gravityBlocks;
	// ボタンブロック
	GimmickGroup<ButtonBlock, MAX_PLATFORMS> buttonBlocks;
	//アイテムブロック
	GimmickGroup<ItemBlock, MAX_PLATFORMS> itemBlocks;
	//カーソルによって動作するギミック
	GimmickGroup<CursorBottom, MAX_PLATFORMS> cursorBottoms;
	//磁石（引き寄せ）
	GimmickGroup<Magnet, MAX_PLATFORMS> magnets;
	//死ぬブロック
	GimmickGroup<Rectangle, MAX_HAZARDS> deathBlocks;
	//壊せるブロック
	GimmickGroup<BreakableBlock, MAX_HAZARDS> breakableBlocks;
	//コメントブロック
	GimmickGroup<CommentBlock, MAX_COMMENT_BLOCKS> commentBlocks;
	// 一時的に消える床
	GimmickGroup<TempFloor, MAX_TEMP_FLOORS> tempFloors;
	// 一時的に消える床のスイッチ
	GimmickGroup<TempFloorSwitch, MAX_TEMP_FLOORS> tempFloorSwitches;
	//デコレーション矢印
	GimmickGroup<DecorArrow, MAX_DECOR_ARROWS> decorArrows;
	//透明ブロック
	GimmickGroup<ClearBlock, MAX_CLEAR> clearBlocks;
	//X方向透明ブロック
	GimmickGroup<ClearBlockX, MAX_CLEAR> clearBlocksX;
	//落下文字
	GimmickGroup<FallingText, MAX_FALLINGTEXT> fallingTexts;
	//スパイク型トランポリン
	GimmickGroup<SpikeBouncer, MAX_SPIKE_BOUNCERS> spikeBouncers;
	//追尾するとげ
	GimmickGroup<TrackingHazard, MAX_HAZARDS> trackingHazards;
	// ================================================================

	//エレベーター
	static constexpr int MAX_ELEVATORS = 8;
	Elevator elevators[MAX_ELEVATORS];
	Elevator elevatorsInit[MAX_ELEVATORS];
	int elevatorCount;
	bool playerInElevator = false; //プレイヤーがエレベーター内か

	bool gravityReversed = false; //重力反転中か
	float gravityCooldown = 0.0f; //重力反転の再発動クールダウン

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
	//スパイク型トランポリン
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

	//スイッチによって動く床
	SwitchPlatform switchPlatforms[MAX_PLATFORMS];
	SwitchPlatform switchplatformsInit[MAX_PLATFORMS];
	int switchPlatformCount;


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
	static constexpr int MAX_SAVEPOINT = 16;
	savePoint savePoints[MAX_SAVEPOINT];
	savePoint savePointsInit[MAX_SAVEPOINT];
	bool isSaved;
	int savePointCount = 0;

};
	
