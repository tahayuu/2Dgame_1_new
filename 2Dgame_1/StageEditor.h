#pragma once
#include "raylib.h"
#include "StageTypes.h"
#include <vector>
#include <string>

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
	COUNT,                  // 50
};

static constexpr int MAX_OBJ_PARAMS = 6;

struct PlacedObject {
    EditorObjectType type;
    Rectangle rect;
    float params[MAX_OBJ_PARAMS] = {};
    std::string text;  // コメントブロック等で使用するテキスト
};

// --- Texture Paint用 ---
struct TextureTile {
    int id = -1;
    Rectangle sourceRect = {};
    std::string texturePath;
    Texture2D texture = {};
};

struct TexturePaintCell {
    int tileId = -1;   // -1: empty
    Vector2 position = {};
};

struct TexturePaintLayer {
    std::vector<TexturePaintCell> cells;
    float cellSize = 50.0f;
    int gridWidth = 256;
    int gridHeight = 256;
    bool visible = true;
    bool locked = false;
    std::string name;
};

struct StageEditor {
    bool active = false;
    EditorObjectType currentType = EditorObjectType::PLATFORM;

    std::vector<PlacedObject> objects;
    std::vector<PlacedObject> undoStack;

    // --- Texture Paint状態 ---
    enum class EditorMode { OBJECT, TEXTURE_PAINT };
    EditorMode currentMode = EditorMode::OBJECT;
    std::vector<TextureTile> textureTiles;
    std::vector<TexturePaintLayer> texturePaintLayers;
    int currentTextureLayerIdx = 0;
    int selectedTileId = -1;
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
};

void EditorInit(StageEditor& ed, int screenWidth, int screenHeight, Font uiFont);
void EditorUpdate(StageEditor& ed, float dt);
void EditorDraw(const StageEditor& ed);
void EditorDrawUI(const StageEditor& ed);
void EditorExportToStage(const StageEditor& ed, Stage& stage);
void EditorImportFromStage(StageEditor& ed, const Stage& stage);
bool EditorSaveJSON(const StageEditor& ed, const char* filename);
bool EditorSaveCSV(const StageEditor& ed, const char* filename);
bool EditorLoadCSV(StageEditor& ed, const char* filename);

// Texture Paint API
void TexturePaintInit(StageEditor& ed, const std::string& textureAssetPath);
void TexturePaintUpdate(StageEditor& ed, float dt);
void TexturePaintDraw(const StageEditor& ed);
void TexturePaintDrawUI(const StageEditor& ed);
void TexturePaintLayerInit(TexturePaintLayer& layer, float cellSize, int gridW, int gridH);
void TexturePaintAddTile(TexturePaintLayer& layer, Vector2 worldPos, int tileId, float cellSize);
void TexturePaintRemoveTile(TexturePaintLayer& layer, Vector2 worldPos, float cellSize);