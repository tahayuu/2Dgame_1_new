#pragma once
#include "raylib.h"
#include "StageTypes.h"
#include "Enemy.h"
#include "EnemyManager.h"
#include <vector>
#include <string>
#include "SpriteDatabase.h" 

// ================================================================
// StageEditor.h の役割
// ---------------------------------------------------------------
// ・ステージエディタで使う「配置データ」「見た目データ」「UI状態」を定義する。
// ・ここで定義したデータは、EditorUpdate/Draw で編集され、
//   EditorExportToStage / EditorImportFromStage / CSV保存読込で受け渡される。
// ・とくに type(当たり判定・挙動) と spriteId(見た目) を分離して持つ設計が要点。
// ================================================================

// StageTypes.h gimmick names
enum class EditorObjectType {
    PLATFORM,               // 0
    BACK_PLATFORM,          // 1
    HAZARD,                 // 2
    TOUCH_BREAK_BLOCK,      // 3
    BOTTOM_BREAK_BLOCK,     // 4
    BREAKABLE_BLOCK,        // 5
    ELEVATOR,               // 6
    GRAVITY_BLOCK,          // 7
    BUTTON_BLOCK,           // 8
    ITEM_BLOCK,             // 9
    ICE_PLATFORM,           // 10
    MOVE_DOWN_PLATFORM,     // 11
    MOVE_UP_PLATFORM,       // 12
    JUMP_PLATFORM,          // 13
    BATTERY_HUMAN,          // 14
    CURSOR_PLATFORM,        // 15
    MAGNET,                 // 16
    KNOCKBACK_WALL,         // 17
    SPLIT_PLATFORM,         // 18
    CIRCLE_PLATFORM,        // 19
    MOVE_PLATFORM_X,        // 20
    MOVE_HAZARD,            // 21
    MOVE_HAZARD_EXT_Y,      // 22
    MOVE_DOWN_HAZARD_EXT_Y, // 23
    MOVE_HAZARD_RIGHT_X,    // 24
    MOVE_HAZARD_EXT_X,      // 25
    TRACKING_HAZARD,        // 26
    ROTATING_BALL,          // 27
    MOVE_ROTATING_BALL,     // 28
    ROLLING_BALL,           // 29
    FALLING_PLATFORM,       // 30
    UPRISING_PLATFORM,      // 31
    UP_DOWN_PLATFORM,       // 32
    CLEAR_BLOCK,            // 33
    CLEAR_BLOCK_X,          // 34
    SWITCH_PLATFORM,        // 35
	FALLING_TEXT,           // 36
	EXIT_DOOR,              // 37
	LAYER_DOOR,             // 38
	RESPAWN,                // 39
	SWITCH_BUTTON,          // 40
	COMMENT_BLOCK,          // 41
	CURSOR_BOTTOM,          // 42
	DEATH_BLOCK,            // 43
	SPIKE_BOUNCER,          // 44
	SPRING,                 // 45
	CRANE_LAUNCH_PAD,       // 46
	CRANE,                  // 47
	OJISAN_PUNCH_AREA,      // 48
	WARP_HOLE,              // 49
    ENEMY,                  // 50
	TEMP_FLOOR,             // 51
	TEMP_FLOOR_SWITCH,      // 52
    DECOR_ARROW,            // 53
	DECOR_SPRITE,           // 54
	COUNT,                  // 55
};

// params[p0..p5] の最大スロット数。
// 各ギミックで使う意味は StageEditor.cpp の TYPE_PARAMS で定義する。
static constexpr int MAX_OBJ_PARAMS = 6;
//=====================================
// 敵配置データ構造
//=====================================
struct PlacedEnemy {// 敵を配置するための構造体
	EnemyType type;                     // 敵の種類（WALKERなど）
	Vector2 pos;                        // エディタ上での配置位置
	float params[MAX_OBJ_PARAMS] = {};// 敵ごとの追加設定（速度・巡回距離など）
};

// エディタ上に置いた「1つの配置物」の保存単位。
// rect/type はゲーム挙動に効く本体データ、sprite系は見た目専用データ。
struct PlacedObject {
	EditorObjectType type;              // 何のギミックとして動作するか（当たり判定・挙動側）
	Rectangle rect;                     // 配置位置と当たり判定サイズ
	float params[MAX_OBJ_PARAMS] = {};  // ギミック個別設定（意味は type ごとに異なる）
	std::string text;  // コメントブロック等で使用するテキスト 

	// ================================================================
	// ここから「見た目専用」のデータ（当たり判定・ギミック処理には使わない）
	// EditorObjectType はギミック(当たり判定・挙動)、
	// spriteId 以下は見た目(描画)だけを担当する。
	//
	// 重要:
	// ・これらは CSV保存/読込で保持される（Sprite見た目を再編集できるため）。
	// ・ゲーム中は Stage 側の spriteInstances に変換して描画で利用する。
	// ・type を変えずに見た目だけ差し替えられるため、
	//   「同じ挙動で別デザイン」を作りやすい。
	// ================================================================
	SpriteId spriteId = SpriteId::None; // 描画に使う画像パーツ（Noneなら今まで通りの仮描画）
	float rotation = 0.0f;              // 見た目の追加回転角度（度）。rect(当たり判定)には影響しない
	bool flipX = false;                 // 左右反転して描画するか
	bool flipY = false;                 // 上下反転して描画するか
};

// --- Texture Paint用 ---
struct TextureTile {
	int id = -1;// タイルID（-1は未使用）
	Rectangle sourceRect = {};// 画像の切り出し位置とサイズ
	std::string texturePath;// 画像ファイルのパス
	Texture2D texture = {};// 実際のテクスチャデータ
};

struct TexturePaintCell {
    int tileId = -1;   // -1: empty
	Vector2 position = {};// ワールド座標での位置
};

struct TexturePaintLayer {
	std::vector<TexturePaintCell> cells;// 塗りつぶしセルのリスト
	float cellSize = 50.0f;// セル1つのサイズ（ワールド座標系）
	int gridWidth = 256;// グリッドの幅（セル数）
	int gridHeight = 256;// グリッドの高さ（セル数）
	bool visible = true;// レイヤーの表示ON/OFF
	bool locked = false;// レイヤーの編集ロックON/OFF
	std::string name;// レイヤー名
};

struct StageEditor {
	bool active = false;// エディタが有効かどうか
	EditorObjectType currentType = EditorObjectType::PLATFORM;// 現在選択中のギミックタイプ


    std::vector<PlacedObject> objects;   // 現在編集中の配置一覧（EditorExportToStage の入力元）
    std::vector<PlacedObject> undoStack; // 削除取り消し用の簡易履歴

    // --- Texture Paint状態 ---
    enum class EditorMode { OBJECT, TEXTURE_PAINT };
    EditorMode currentMode = EditorMode::OBJECT; // OBJECT=ギミック配置 / TEXTURE_PAINT=見た目タイル塗り
    std::vector<TextureTile> textureTiles;       // パレットに並べる候補画像
    std::vector<TexturePaintLayer> texturePaintLayers; // 塗りレイヤー（表示ON/OFF単位）
    int currentTextureLayerIdx = 0;              // 編集対象のレイヤー番号
    int selectedTileId = -1;                     // 現在選択中のタイルID（-1は未選択）
    bool showTilePalette = false;
    Vector2 tilePaletteScroll = { 0, 0 };
    float brushSize = 1.0f;
    // ------------------------

    float gridSize = 50.0f;
    int   gridW = 2;
    int   gridH = 1;

    Camera2D camera = {};
    float scrollSpeed = 600.0f;
    bool isDragging = false;
    Vector2 dragStart = {};
    // 保存先のベース名。実ファイルは savePath + ".csv" / ".json" で扱う。
    std::string savePath = "stage_editor_output";

    // Toolbar / UI
    Font uiFont = {};
    int  screenW = 1280;
    int  screenH = 720;
    float saveNotifyTimer = 0.0f;
    int  toolbarOffset = 0;
    static constexpr int   VISIBLE_BTNS = 9;
    static constexpr float TOOLBAR_H    = 74.0f;
    static constexpr float BTN_W        = 118.0f;
    static constexpr float BTN_H        = 58.0f;
    static constexpr float BTN_GAP      = 5.0f;
    static constexpr float BTN_PAD_TOP  = 8.0f;
    static constexpr float BOTTOM_H     = 34.0f;

    // Property panel (右クリックで開くパラメータ編集パネル)
    int propSelectedIdx = -1;       // 選択中のオブジェクト (-1=なし)
    int propEditingParam = -1;      // 編集中のパラメータ (-1=なし)
    char propEditBuf[32] = {};      // テキスト入力バッファ
    int propEditCursor = 0;         // カーソル位置


    // テキスト編集 (コメントブロック用)
    bool propEditingText = false;   // テキスト編集モード中か
    char propTextBuf[256] = {};     // テキスト入力バッファ
    int propTextCursor = 0;         // カーソル位置

	//=================
	//敵配置機能
	//=================

	std::vector<PlacedEnemy> placedEnemies;// 配置された敵のリスト
	EnemyType currentEnemyType = EnemyType::WALKER;  
	int selectedEnemyIdx = -1; // 選択中の敵インデックス

	//敵UI状態
	bool enemyMenuOpen = false; // 敵配置メニューが開いているか
	Rectangle enemyMenuRect = {};// 敵配置メニューの矩形
	int hoveredEnemyType = -1;// ホバー中の敵タイプインデックス

	// ================================================================
	// コピー＆ペースト機能（クリップボード）
	// ================================================================
	PlacedObject clipboard = {}; // コピーしたオブジェクトを一時保存
	bool hasClipboard = false;   // クリップボードに有効なデータがあるか
};

// 目的: エディタ状態を編集開始可能な初期値へ揃える。
// 入力: 画面サイズ、UIフォント。
// 出力: ed を初期化し、既存CSVがあれば自動読込する。
// 注意: savePath を変更した場合、起動時に読むCSVの場所も変わる。
void EditorInit(StageEditor& ed, int screenWidth, int screenHeight, Font uiFont);

// 目的: オブジェクト配置・選択・パラメータ編集・保存操作を毎フレーム処理する。
// 入力: 現在の入力状態と経過時間 dt。
// 出力: ed の配置データ/UI状態が更新される。
// 注意: ここでの変更結果が Export/Save の元データになる。
void EditorUpdate(StageEditor& ed, float dt);

// 目的: ワールド側の編集内容（配置物と敵）を可視化する。
// 入力: 現在のエディタ状態。
// 出力: 画面描画のみ（データは変更しない）。
void EditorDraw(const StageEditor& ed);

// 目的: ツールバー・ガイド・プロパティパネルなど編集UIを描画する。
// 入力: ed（必要に応じてパネル選択状態も参照）。
// 出力: UI描画のみ。
void EditorDrawUI(StageEditor& ed);

// 目的: PlacedObject/PlacedEnemy を実行時 Stage/EnemyManager 形式へ変換する。
// 入力: ed 内の配置データ。
// 出力: stage と enemyManager がゲーム実行用データに再構築される。
// 注意: type は挙動側、spriteId は見た目側として別経路で反映する。
void EditorExportToStage(const StageEditor& ed, Stage& stage, EnemyManager& enemyManager);

// 目的: 既存 Stage をエディタ編集可能な PlacedObject 群へ復元する。
// 入力: 実行時 Stage データ。
// 出力: ed.objects / ed.placedEnemies に編集用データが入る。
// 注意: Stage側の spriteInstances と rect一致で見た目情報を復元する。
void EditorImportFromStage(StageEditor& ed, const Stage& stage);

// 目的: 配置内容をJSONとして外部保存する。
// 入力: ed と保存先ファイル名。
// 出力: 保存成功時 true。
// 注意: JSON読込は未実装のため、現状は確認/バックアップ用途。
bool EditorSaveJSON(const StageEditor& ed, const char* filename);

// 目的: 編集データの標準保存形式としてCSVを書き出す。
// 入力: ed と保存先ファイル名。
// 出力: 保存成功時 true。
// 注意: p0〜p5・text・sprite見た目情報も含めて保存する。
bool EditorSaveCSV(const StageEditor& ed, const char* filename);

// 目的: CSVからエディタ状態を復元する。
// 入力: 読み込むCSVファイル名。
// 出力: 読込成功時 true。
// 注意: 古いCSV（sprite列なし）も読めるよう後方互換を持たせている。
bool EditorLoadCSV(StageEditor& ed, const char* filename);

// Texture Paint API
// テクスチャペイント機能を初期化し、タイル一覧を読み込む
void TexturePaintInit(StageEditor& ed, const std::string& textureAssetPath);
// テクスチャペイントモードの入力・塗り処理を毎フレーム更新する
void TexturePaintUpdate(StageEditor& ed, float dt);
// テクスチャペイント結果（配置済みタイル）をワールドに描画する
void TexturePaintDraw(const StageEditor& ed);
// タイルパレットやレイヤー情報など、テクスチャペイント用UIを描画する
void TexturePaintDrawUI(const StageEditor& ed);
// ペイント用レイヤーを指定サイズで初期化する
void TexturePaintLayerInit(TexturePaintLayer& layer, float cellSize, int gridW, int gridH);
// 指定座標のセルにタイルを配置する
void TexturePaintAddTile(TexturePaintLayer& layer, Vector2 worldPos, int tileId, float cellSize);
// 指定座標のセルからタイルを削除する
void TexturePaintRemoveTile(TexturePaintLayer& layer, Vector2 worldPos, float cellSize);
