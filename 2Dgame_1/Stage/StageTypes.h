float stageHeight;
bool playerOnUpRising = false;

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

// クレーンとクレーン発射台に使う共通上限
static constexpr int MAX_CRANES = 8;

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

// 0：SPACE/W/UP
// 1：Nのみ
int currentJumpMode = 0;
static constexpr int MAX_SAVEPOINT = 16;
savePoint savePoints[MAX_SAVEPOINT];
savePoint savePointsInit[MAX_SAVEPOINT];
bool isSaved;
int savePointCount = 0;

// ================================================================
// GimmickGroup に移行済みの全ギミック（34種類）
// 種類ごとに定義を集約。実体・初期状態・登録数を各グループが保持する。
// ================================================================
// ブロック・スイッチ系
GimmickGroup<TouchBreakBlock, MAX_PLATFORMS> touchBreakBlocks;
GimmickGroup<BottomBreakBlock, MAX_HAZARDS> bottomBreakBlocks;
GimmickGroup<GravityBlock, MAX_PLATFORMS> gravityBlocks;
GimmickGroup<ButtonBlock, MAX_PLATFORMS> buttonBlocks;
GimmickGroup<ItemBlock, MAX_PLATFORMS> itemBlocks;
GimmickGroup<Rectangle, MAX_HAZARDS> deathBlocks;
GimmickGroup<BreakableBlock, MAX_HAZARDS> breakableBlocks;
GimmickGroup<ClearBlock, MAX_CLEAR> clearBlocks;
GimmickGroup<ClearBlockX, MAX_CLEAR> clearBlocksX;
GimmickGroup<CommentBlock, MAX_COMMENT_BLOCKS> commentBlocks;

// 床・移動ギミック系
GimmickGroup<TempFloor, MAX_TEMP_FLOORS> tempFloors;
GimmickGroup<TempFloorSwitch, MAX_TEMP_FLOORS> tempFloorSwitches;
GimmickGroup<IcePlatform, MAX_PLATFORMS> icePlatforms;
GimmickGroup<MoveDownPlatform, MAX_PLATFORMS> moveDownPlatforms;
GimmickGroup<JumpPlatfrom, MAX_PLATFORMS> jumpPlatfroms;
GimmickGroup<KnockBackWall, MAX_PLATFORMS> knockBackWalls;
GimmickGroup<FallingPlatform, MAX_FALLING> fallingPlatforms;
GimmickGroup<UpRisingPlatform, MAX_UPRISING> upRisingPlatforms;
GimmickGroup<UpDownPlatform, MAX_UPRISING> upDouwnPlatforms;
GimmickGroup<CraneLaunchPad, MAX_CRANES> craneLaunchPads;
GimmickGroup<SpikeBouncer, MAX_SPIKE_BOUNCERS> spikeBouncers;

// トゲ・鉄球系
GimmickGroup<MoveHazard, MAX_MOVEHAZARD> moveHazards;
GimmickGroup<MoveHazardExtentionY, MAX_MOVEHAZARD> moveHazardsExtY;
GimmickGroup<MoveDownHazardExtentionY, MAX_MOVEHAZARD> moveDownHazardsExtY;
GimmickGroup<MoveHazardRightX, MAX_MOVEHAZARD> moveHazardsRight;
GimmickGroup<TrackingHazard, MAX_HAZARDS> trackingHazards;
GimmickGroup<RotatingBall, MAX_HAZARDS> rotatingBalls;
GimmickGroup<MoveRotatingBall, MAX_HAZARDS> moveRotatingBalls;
GimmickGroup<RollingBall, MAX_HAZARDS> rollingBalls;

// その他（演出・連動・装飾）
GimmickGroup<CursorBottom, MAX_PLATFORMS> cursorBottoms;
GimmickGroup<Magnet, MAX_PLATFORMS> magnets;
GimmickGroup<FallingText, MAX_FALLINGTEXT> fallingTexts;
GimmickGroup<DecorArrow, MAX_DECOR_ARROWS> decorArrows;
GimmickGroup<EventChanger, MAX_EVENT_CHANGERS> eventChangers;
// ================================================================

};

