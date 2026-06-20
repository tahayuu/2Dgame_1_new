#include "StageEditorInternal.h"

// ================================================================
// パラメータテーブル
// ================================================================
static const TypeParamInfo TYPE_PARAMS[(int)EditorObjectType::COUNT] = {
    /* 0  PLATFORM              */ { 0, {} },
    /* 1  BACK_PLATFORM         */ { 0, {} },
    /* 2  HAZARD                */ { 0, {} },
    /* 3  TOUCH_BREAK_BLOCK     */ { 0, {} },
    /* 4  BOTTOM_BREAK_BLOCK    */ { 0, {} },
    /* 5  BREAKABLE_BLOCK       */ { 0, {} },
    /* 6  ELEVATOR              */ { 3, {{"speed",200,false},{"rangeUp",200,false},{"rangeDn",0,false}} },
    /* 7  GRAVITY_BLOCK         */ { 0, {} },
    /* 8  BUTTON_BLOCK          */ { 0, {} },
    /* 9  ITEM_BLOCK            */ { 0, {} },
    /* 10 ICE PLATFORM          */ { 3, {{"friction",0.02f,false},{"iceAccel",400,false},{"maxSlide",500,false}} },
    /* 11 MOVE_DOWN_PLATFORM    */ { 0, {} },
    /* 12 MOVE_UP_PLATFORM      */ { 0, {} },
    /* 13 JUMP PLATFORM         */ { 6, {{"jumpVelX",0,false},{"jumpVelY",-800,false},{"stopTm",0.4f,false},{"delay",0.1f,false},{"withdraw",3.0f,false},{"jumpSpd",200,false}} },
    /* 14 BATTERY_HUMAN         */ { 3, {{"velX",800,false},{"velY",-400,false},{"delay",0.5f,false}} },
    /* 15 CURSOR_PLATFORM       */ { 2, {{"followSpd",200,false},{"maxDist",300,false}} },
    /* 16 MAGNET                */ { 2, {{"Force",500,false},{"maxDist",200,false}} },
    /* 17 KNOCKBACK_WALL        */ { 5, {{"knockVelX",-600,false},{"knockVelY",-200,false},{"withdraw",5.0f,false},{"delay",0.1f,false},{"limitStop",0.1f,false}} },
    /* 18 SPLIT_PLATFORM        */ { 2, {{"maxGap",100,false},{"splitSpd",50,false}} },
    /* 19 CIRCLE_PLATFORM       */ { 2, {{"radius",20,false},{"angleSpd",1.5f,false}} },
    /* 20 MOVE_PLATFORM_X       */ { 3, {{"moveSpd",200,false},{"moveDist",200,false},{"delay",0,false}} },
    /* 21 MOVE_HAZARD           */ { 2, {{"raiseH",100,false},{"moveSpd",200,false}} },
    /* 22 MOVE_HAZARD_EXT_Y     */ { 3, {{"raiseH",100,false},{"moveSpd",200,false},{"dir(1or-1)",1,false}} },
    /* 23 MOVE_DOWN_HAZ_EXT_Y   */ { 3, {{"raiseH",100,false},{"moveSpd",200,false},{"tolerance",5.0f,false}} },
    /* 24 MOVE_HAZARD_RIGHT_X   */ { 3, {{"raiseW",100,false},{"moveSpd",200,false},{"delay",0,false}} },
    /* 25 MOVE_HAZARD_EXT_X     */ { 3, {{"raiseW",100,false},{"moveSpd",200,false},{"delay",0,false}} },
    /* 26 TRACKING_HAZARD       */ { 4, {{"speed",150,false},{"trkRange",300,false},{"maxDist",400,false},{"retStart",1,true}} },
    /* 27 ROTATING_BALL         */ { 3, {{"radius",15,false},{"angSpd",2,false},{"armLen",100,false}} },
    /* 28 MOVE_ROTATING_BALL    */ { 5, {{"radius",15,false},{"angSpd",2,false},{"gravity",1600,false},{"reverse",0,true},{"armLen",100,false}} },
    /* 29 ROLLING_BALL */ { 4, {{"radius",15,false},{"rollSpd",200,false},{"rollDir",-1,false},{"waitBtn",1,true}} },
    /* 30 FALLING_PLATFORM */ { 3, {{"fallDelay",0.5f,false},{"fallSpd",400,false},{"detect(0:both 1:p 2:b)",0,false}} },
    /* 31 UPRISING_PLATFORM     */ { 2, {{"riseDelay",0.3f,false},{"riseSpd",300,false}} },
    /* 32 UP_DOWN_PLATFORM      */ { 4, {{"riseH",150,false},{"riseSpd",200,false},{"fallSpd",200,false},{"stopTm",0.5f,false}} },
    /* 33 CLEAR_BLOCK           */ { 0, {} },
    /* 34 CLEAR_BLOCK_X         */ { 0, {} },
    /* 35 SWITCH_PLATFORM       */ { 3, {{"speedX",100,false},{"speedY",0,false},{"maxX",200,false}} },
    /* 36 FALLING_TEXT          */ { 4, {{"fallDelay",0.2f,false},{"gravity",1600,false},{"tolerance",300,false},{"fontSize",48,false}} },
    /* 37 EXIT_DOOR             */ { 1, {{"targetStage",1,false}} },
    /* 38 LAYER_DOOR            */ { 0, {} },
    /* 39 RESPAWN               */ { 0, {} },
    /* 40 SWITCH_BUTTON         */ { 0, {} },
    /* 41 COMMENT_BLOCK         */ { 1, {{"duration",3.0f,false}} }, 
    /* 42 CURSOR_BOTTOM         */ { 3, {{"targetBall",-1,false},{"oneShot",1,true},{"maxDist",0,false}} }, // 追加
	/* 43 DEATH_BLOCK           */ { 0, {} }, 
    /* 44 SPIKE_BOUNCER         */ { 2, {{"bounceVelX",0,false},{"bounceVelY",-600,false}} },
    /* 45 SPRING                */ { 2, {{"bounceVelX",0,false},{"bounceVelY",-800,false}} },
    /* 46 CRANE_LAUNCH_PAD */ { 1, {{"launchVelY",-900,false}} },
    /* 47 CRANE */ { 5, {{"maxArmLen",200,false},{"detectRangeY",600,false},{"carrySpeedX",250,false},{"carryDir",1,false},{"carryDist",2000,false}}},
    /* 48 OJISAN_PUNCH_AREA     */ { 0, {} },
};

const TypeParamInfo& EdGetTypeInfo(EditorObjectType t) {
    return TYPE_PARAMS[(int)t];
}

void InitDefaultParams(PlacedObject& obj) {
    const auto& info = TYPE_PARAMS[(int)obj.type];
    for (int i = 0; i < MAX_OBJ_PARAMS; i++)
        obj.params[i] = (i < info.count) ? info.defs[i].defaultValue : 0.0f;
}

Rectangle GetPropPanelRect(const StageEditor& ed) {
    int pc = 0;
    bool isComment = false;
    if (ed.propSelectedIdx >= 0 && ed.propSelectedIdx < (int)ed.objects.size()) {
        const auto& obj = ed.objects[ed.propSelectedIdx];
        pc = EdGetTypeInfo(obj.type).count;
        isComment = (obj.type == EditorObjectType::COMMENT_BLOCK);
    }
    float rows = (pc > 0) ? (float)pc : 1.0f;
    float px = ed.screenW - PROP_W - 10.0f;
    float py = ed.TOOLBAR_H + 10.0f;
    float h = PROP_HEADER_H + rows * PROP_LINE_H + 48.0f;
    if (isComment) {
        h += PROP_LINE_H + 4.0f;  // text 入力行
        h += PROP_LINE_H;          // @キー プレビュー行
    }
    return { px, py, PROP_W, h };
}

// ================================================================
// 名前・カラーテーブル
// ================================================================
static const char* const EN_NAMES[(int)EditorObjectType::COUNT] = {
    "platforms","backPlatforms","hazards","touchBreakBlocks","bottomBreakBlocks",
    "breakableBlocks","elevators","gravityBlocks","buttonBlocks","itemBlocks",
    "icePlatforms","moveDownPlatforms","moveUpPlatforms","jumpPlatfroms","batteryHumans",
    "cursorPlatforms","magnets","knockBackWalls","splitPlatforms","circlePlatforms",
    "movePlatformsX","moveHazards","moveHazardsExtY","moveDownHazardsExtY","moveHazardsRight",
    "moveHazardsExtX","trackingHazards","rotatingBalls","moveRotatingBalls","rollingBalls",
    "fallingPlatforms","upRisingPlatforms","upDouwnPlatforms","clearBlocks","clearBlocksX",
    "switchPlatforms","fallingTexts","exitDoors","layerDoors","respawn","switchButtons",
    "commentBlocks","cursorButtons","deathBlocks","spikeBouncers","springs",
    "craneLaunchPads","cranes","ojisanPunchAreas"
};

static const char8_t* const JP_NAMES_U8[(int)EditorObjectType::COUNT] = {
    u8"platforms",u8"backPlatforms",u8"hazards",u8"touchBreakBlocks",u8"bottomBreakBlocks",
    u8"breakableBlocks",u8"elevators",u8"gravityBlocks",u8"buttonBlocks",u8"itemBlocks",
    u8"icePlatforms",u8"moveDownPlatforms",u8"moveUpPlatforms",u8"jumpPlatfroms",u8"batteryHumans",
    u8"cursorPlatforms",u8"magnets",u8"knockBackWalls",u8"splitPlatforms",u8"circlePlatforms",
    u8"movePlatformsX",u8"moveHazards",u8"moveHazardsExtY",u8"moveDownHazardsExtY",u8"moveHazardsRight",
    u8"moveHazardsExtX",u8"trackingHazards",u8"rotatingBalls",u8"moveRotatingBalls",u8"rollingBalls",
    u8"fallingPlatforms",u8"upRisingPlatforms",u8"upDouwnPlatforms",u8"clearBlocks",u8"clearBlocksX",
    u8"switchPlatforms",u8"fallingTexts",u8"exitDoors",u8"layerDoors",u8"respawn",u8"switchButtons",
    u8"commentBlocks",u8"cursorButtons",u8"deathBlocks",u8"spikeBouncers",u8"springs",
    u8"craneLaunchPads",u8"cranes",u8"ojisanPunchAreas"
};

const char* GetNameJP(int i) {
    if (i < 0 || i >= (int)EditorObjectType::COUNT) return "???";
    return reinterpret_cast<const char*>(JP_NAMES_U8[i]);
}
const char* GetNameEN(int i) {
    if (i < 0 || i >= (int)EditorObjectType::COUNT) return "Unknown";
    return EN_NAMES[i];
}

static const Color TYPE_COLORS[(int)EditorObjectType::COUNT] = {
    BEIGE,                  //  0 Platform
    {180,180,180,255},      //  1 BackPlatform
    RED,                    //  2 Hazard
    {200,150,100,255},      //  3 TouchBreakBlock
    {180,130, 90,255},      //  4 BottomBreakBlock
    BROWN,                  //  5 BreakableBlock
    {100,100,200,255},      //  6 Elevator
    PURPLE,                 //  7 GravityBlock
    {220,100,100,255},      //  8 ButtonBlock
    YELLOW,                 //  9 ItemBlock
    SKYBLUE,                // 10 IcePlatform
    {150,120, 80,255},      // 11 MoveDownPlatform
    {120,180,120,255},      // 12 MoveUpPlatform
    LIME,                   // 13 JumpPlatform
    {140,140,140,255},      // 14 BatteryHuman
    {180,180,220,255},      // 15 CursorPlatform
    {200, 50, 50,255},      // 16 Magnet
    {100, 60, 60,255},      // 17 KnockBackWall
    {180,160,100,255},      // 18 SplitPlatform
    {100,180,180,255},      // 19 CirclePlatform
    {180,150,120,255},      // 20 MovePlatformX
    MAROON,                 // 21 MoveHazard
    {160, 40, 40,255},      // 22 MoveHazardExtY
    {140, 30, 30,255},      // 23 MoveDownHazardExtY
    {180, 50, 50,255},      // 24 MoveHazardRightX
    {150, 40, 40,255},      // 25 MoveHazardExtX
    {200, 60, 60,255},      // 26 TrackingHazard
    DARKGRAY,               // 27 RotatingBall
    { 80, 80, 80,255},      // 28 MoveRotatingBall
    {100,100,100,255},      // 29 RollingBall
    ORANGE,                 // 30 FallingPlatform
    {220,180,100,255},      // 31 UpRisingPlatform
    {200,170, 90,255},      // 32 UpDownPlatform
    {255,255,255,100},      // 33 ClearBlock
    {240,240,255,100},      // 34 ClearBlockX
    {100,200,100,255},      // 35 SwitchPlatform
    {200,200,100,255},      // 36 FallingText
    {180,100,220,255},      // 37 ExitDoor
    {160, 80,200,255},      // 38 LayerDoor
    GOLD,                   // 39 Respawn
		{220, 140,  60, 255},   // 40 SwitchButton（オレンジ）
  { 60, 200, 200, 255},   // 41 CommentBlock（シアン）
	{255,200,0,255},	  // 42 CursorBottom（黄色）
	{255, 40, 140, 255},      // 43 DeathBlock
	   {200, 80, 200, 255},      // 44 SpikeBouncer（紫/マゼンタ）
		   {255, 192, 203, 255},     // 45 Spring（ピンク）
	{255, 220,   0, 255},     // 46 CraneLaunchPad（明るい黄色）
	{120, 120, 120, 255},     // 47 Crane（濃いグレー）
	{255, 120, 120, 255},   // 48 OjisanPunchArea
};

Color GetColor(int i) {
    if (i < 0 || i >= (int)EditorObjectType::COUNT) return GRAY;
    return TYPE_COLORS[i];
}

// ================================================================
// UIヘルパー
// ================================================================
std::string FitLabelToWidth(const char* text, int fontSize, float maxWidth) {
    std::string s = text ? text : "";
    if (MeasureText(s.c_str(), fontSize) <= maxWidth) return s;
    while (!s.empty()) {
        s.pop_back();
        std::string t = s + "...";
        if (MeasureText(t.c_str(), fontSize) <= maxWidth) return t;
    }
    return "...";
}

Vector2 SnapToGrid(Vector2 world, float grid) {
    return { floorf(world.x / grid) * grid, floorf(world.y / grid) * grid };
}

Rectangle GetBtnRect(const StageEditor& ed, int visibleIdx) {
    float totalW = ed.VISIBLE_BTNS * ed.BTN_W + (ed.VISIBLE_BTNS - 1) * ed.BTN_GAP;
    float sx = (ed.screenW - totalW) / 2.0f;
    return { sx + visibleIdx * (ed.BTN_W + ed.BTN_GAP), ed.BTN_PAD_TOP, ed.BTN_W, ed.BTN_H };
}

void AdjustToolbarOffset(StageEditor& ed) {
    int cur = (int)ed.currentType;
    int maxOff = (int)EditorObjectType::COUNT - ed.VISIBLE_BTNS;
    if (maxOff < 0) maxOff = 0;
    if (cur < ed.toolbarOffset) ed.toolbarOffset = cur;
    if (cur >= ed.toolbarOffset + ed.VISIBLE_BTNS)
        ed.toolbarOffset = cur - ed.VISIBLE_BTNS + 1;
    if (ed.toolbarOffset > maxOff) ed.toolbarOffset = maxOff;
    if (ed.toolbarOffset < 0) ed.toolbarOffset = 0;
}

void DrawObjectIcon(int typeIdx, Rectangle r) {
    Color c = GetColor(typeIdx);
    if (typeIdx == 2 || (typeIdx >= 21 && typeIdx <= 26)) {
        DrawRectangleRec(r, ColorAlpha(c, 0.25f));
        DrawTriangle(
            { r.x + 3, r.y + r.height - 2 },
            { r.x + r.width - 3, r.y + r.height - 2 },
            { r.x + r.width / 2, r.y + 3 }, c);
    }
    else if (typeIdx >= 27 && typeIdx <= 29) {
        float cx = r.x + r.width / 2, cy = r.y + r.height / 2;
        float rad = (std::min)(r.width, r.height) / 2 - 2;
        DrawCircle((int)cx, (int)cy, rad, c);
        DrawCircleLines((int)cx, (int)cy, rad, BLACK);
    }
    else if (typeIdx == 33 || typeIdx == 34) {
        DrawRectangleRec(r, ColorAlpha(WHITE, 0.2f));
        DrawRectangleLinesEx(r, 2, ColorAlpha(WHITE, 0.6f));
    }
    else if (typeIdx == 39) {
        float cx = r.x + r.width / 2, cy = r.y + r.height / 2;
        DrawCircle((int)cx, (int)cy, (std::min)(r.width, r.height) / 2 - 2, GOLD);
    }
    else if (typeIdx == 40) {  // SWITCH_BUTTON: ボタン形状
        float cx = r.x + r.width / 2;
        DrawRectangle((int)(cx - 6), (int)(r.y + r.height - 6), 12, 6, {220,140,60,255});
        DrawCircle((int)cx, (int)(r.y + r.height / 2 - 2), 7, {220,140,60,255});
        DrawCircleLines((int)cx, (int)(r.y + r.height / 2 - 2), 7, BLACK);
    }
    else if (typeIdx == 42) { // CURSOR_BOTTOM 追加
        DrawRectangleRec(r, ColorAlpha(c, 0.85f));
        DrawRectangleLinesEx(r, 2, DARKBROWN);
        float cx = r.x + r.width * 0.5f;
        float cy = r.y + r.height * 0.5f;
        DrawCircle((int)cx, (int)cy, 4.0f, BLACK);
    }
    else if (typeIdx == (int)EditorObjectType::OJISAN_PUNCH_AREA) {
        DrawRectangleRec(r, ColorAlpha(c, 0.8f));
        DrawRectangleLinesEx(r, 2, MAROON);
        DrawText("!", (int)(r.x + r.width / 2 - 3), (int)(r.y + 2), 14, BLACK);
    }
    else {
        DrawRectangleRec(r, c);
        DrawRectangleLinesEx(r, 1, ColorAlpha(BLACK, 0.4f));
    }
}

// ================================================================
void EditorInit(StageEditor& ed, int screenWidth, int screenHeight, Font uiFont) {
    ed.active = true;
    ed.objects.clear();
    ed.undoStack.clear();
    ed.currentType = EditorObjectType::PLATFORM;
    ed.camera.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
    ed.camera.target = { screenWidth / 2.0f, screenHeight / 2.0f };
    ed.camera.zoom = 1.0f;
    ed.camera.rotation = 0.0f;
    ed.uiFont = uiFont;
    ed.screenW = screenWidth;
    ed.screenH = screenHeight;
    ed.saveNotifyTimer = 0.0f;
    ed.toolbarOffset = 0;

    std::string csvPath = ed.savePath + ".csv";
    if (FileExists(csvPath.c_str())) {
        EditorLoadCSV(ed, csvPath.c_str());
    }
}

// ================================================================
void EditorUpdate(StageEditor& ed, float dt) {
    if (!ed.active) return;
    if (ed.saveNotifyTimer > 0.0f) ed.saveNotifyTimer -= dt;

    if (ed.propSelectedIdx >= (int)ed.objects.size()) {
        ed.propSelectedIdx = -1;
        ed.propEditingParam = -1;
    }

    const int TC = (int)EditorObjectType::COUNT;

    bool mouseInPropPanel = false;
    if (ed.propSelectedIdx >= 0)
        mouseInPropPanel = CheckCollisionPointRec(GetMousePosition(), GetPropPanelRect(ed));

    // === パラメータ テキスト編集中 ===
    if (ed.propSelectedIdx >= 0 && ed.propEditingParam >= 0) {
        auto& sel = ed.objects[ed.propSelectedIdx];
        int ch = GetCharPressed();
        while (ch > 0) {
            if ((ch >= '0' && ch <= '9') || ch == '.' || ch == '-') {
                if (ed.propEditCursor < 30) {
                    ed.propEditBuf[ed.propEditCursor++] = (char)ch;
                    ed.propEditBuf[ed.propEditCursor] = '\0';
                }
            }
            ch = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && ed.propEditCursor > 0)
            ed.propEditBuf[--ed.propEditCursor] = '\0';
        if (IsKeyPressed(KEY_ENTER)) {
            sel.params[ed.propEditingParam] = (float)atof(ed.propEditBuf);
            ed.propEditingParam = -1;
        }
        if (IsKeyPressed(KEY_ESCAPE))   ed.propEditingParam = -1;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !mouseInPropPanel)
            ed.propEditingParam = -1;
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
            EditorSaveJSON(ed, (ed.savePath + ".json").c_str());
            EditorSaveCSV(ed, (ed.savePath + ".csv").c_str());
            ed.saveNotifyTimer = 2.0f;
        }
        return;
    }

    // === コメントブロック テキスト編集中 ===
    if (ed.propSelectedIdx >= 0 && ed.propEditingText) {
        auto& sel = ed.objects[ed.propSelectedIdx];

        // Ctrl+V: クリップボードからペースト（日本語入力の代替手段）
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)) {
            const char* clip = GetClipboardText();
            if (clip) {
                int ci = 0;
                while (clip[ci] != '\0' && ed.propTextCursor < 254) {
                    ed.propTextBuf[ed.propTextCursor++] = clip[ci++];
                }
                ed.propTextBuf[ed.propTextCursor] = '\0';
            }
        }

        // IME確定キー(Enter)と編集終了キー(Enter)の競合を防ぐフラグ
        bool charAddedThisFrame = false;
        int ch = GetCharPressed();
        while (ch > 0) {
            if (ed.propTextCursor < 250) {
                charAddedThisFrame = true;
                char utf8[5] = {};
                int len = 0;
                if (ch < 0x80) { utf8[0] = (char)ch; len = 1; }
                else if (ch < 0x800) {
                    utf8[0] = (char)(0xC0 | (ch >> 6));
                    utf8[1] = (char)(0x80 | (ch & 0x3F));
                    len = 2;
                }
                else if (ch < 0x10000) {
                    utf8[0] = (char)(0xE0 | (ch >> 12));
                    utf8[1] = (char)(0x80 | ((ch >> 6) & 0x3F));
                    utf8[2] = (char)(0x80 | (ch & 0x3F));
                    len = 3;
                }
                else {
                    utf8[0] = (char)(0xF0 | (ch >> 18));
                    utf8[1] = (char)(0x80 | ((ch >> 12) & 0x3F));
                    utf8[2] = (char)(0x80 | ((ch >> 6) & 0x3F));
                    utf8[3] = (char)(0x80 | (ch & 0x3F));
                    len = 4;
                }
                for (int i = 0; i < len && ed.propTextCursor < 254; i++)
                    ed.propTextBuf[ed.propTextCursor++] = utf8[i];
                ed.propTextBuf[ed.propTextCursor] = '\0';
            }
            ch = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && ed.propTextCursor > 0) {
            int pos = ed.propTextCursor - 1;
            while (pos > 0 && (ed.propTextBuf[pos] & 0xC0) == 0x80) pos--;
            ed.propTextBuf[pos] = '\0';
            ed.propTextCursor = pos;
        }
        // 同フレームで文字入力があった場合(IME確定Enter)は閉じない
        if (IsKeyPressed(KEY_ENTER) && !charAddedThisFrame) {
            sel.text = ed.propTextBuf;
            ed.propEditingText = false;
        }
        if (IsKeyPressed(KEY_ESCAPE)) ed.propEditingText = false;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !mouseInPropPanel)
            ed.propEditingText = false;
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
            sel.text = ed.propTextBuf;
            EditorSaveJSON(ed, (ed.savePath + ".json").c_str());
            EditorSaveCSV(ed, (ed.savePath + ".csv").c_str());
            ed.saveNotifyTimer = 2.0f;
        }
        return;
    }

    // === プロパティパネル内クリック ===
    if (mouseInPropPanel && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Rectangle panel = GetPropPanelRect(ed);
        Vector2 mp = GetMousePosition();
        const auto& info = EdGetTypeInfo(ed.objects[ed.propSelectedIdx].type);
        float paramY = panel.y + PROP_HEADER_H;
        for (int i = 0; i < info.count; i++) {
            Rectangle row = { panel.x + 5, paramY + i * PROP_LINE_H, panel.width - 10, PROP_LINE_H };
            if (CheckCollisionPointRec(mp, row)) {
                if (info.defs[i].isBool) {
                    // boolパラメータ: クリックで 0↔1 トグル
                    float& val = ed.objects[ed.propSelectedIdx].params[i];
                    val = (val != 0.0f) ? 0.0f : 1.0f;
                } else {
                    ed.propEditingParam = i;
                    snprintf(ed.propEditBuf, sizeof(ed.propEditBuf), "%.2f",
                        ed.objects[ed.propSelectedIdx].params[i]);
                    ed.propEditCursor = (int)strlen(ed.propEditBuf);
                }
                return;
            }
        }
        // 削除ボタン
        float delY = paramY + ((info.count > 0) ? info.count : 1) * PROP_LINE_H + 8;
        Rectangle delBtn = { panel.x + panel.width / 2 - 50, delY, 100, 24 };
        if (CheckCollisionPointRec(mp, delBtn)) {
            ed.undoStack.push_back(ed.objects[ed.propSelectedIdx]);
            ed.objects.erase(ed.objects.begin() + ed.propSelectedIdx);
            ed.propSelectedIdx = -1;
            return;
        }

        // テキスト行クリック（コメントブロック用）
        if (ed.objects[ed.propSelectedIdx].type == EditorObjectType::COMMENT_BLOCK) {
            float textRowY = paramY + ((info.count > 0) ? info.count : 1) * PROP_LINE_H;
            Rectangle textRow = { panel.x + 5, textRowY, panel.width - 10, PROP_LINE_H };
            if (CheckCollisionPointRec(mp, textRow)) {
                ed.propEditingText = true;
                const std::string& curText = ed.objects[ed.propSelectedIdx].text;
                snprintf(ed.propTextBuf, sizeof(ed.propTextBuf), "%s", curText.c_str());
                ed.propTextCursor = (int)strlen(ed.propTextBuf);
                return;
            }
        }
    }

    // === 通常のエディタ入力 ===
    for (int k = KEY_ONE; k <= KEY_NINE; k++) {
        int idx = k - KEY_ONE;
        if (idx < TC && IsKeyPressed(k)) ed.currentType = (EditorObjectType)idx;
    }
    if (IsKeyPressed(KEY_TAB))
        ed.currentType = (EditorObjectType)(((int)ed.currentType + 1) % TC);

    static const float kGridSizes[] = { 10.0f, 20.0f, 40.0f, 80.0f };
    static const int   kGridSizeCount = 4;
    if (IsKeyPressed(KEY_R)) {
        int nextIdx = 0;
        for (int i = 0; i < kGridSizeCount; i++) {
            if (fabsf(kGridSizes[i] - ed.gridSize) < 0.1f) { nextIdx = (i + 1) % kGridSizeCount; break; }
        }
        ed.gridSize = kGridSizes[nextIdx];
    }
    if (!IsKeyDown(KEY_LEFT_CONTROL)) {
        if (IsKeyPressed(KEY_Q) && ed.gridW > 1) ed.gridW--;
        if (IsKeyPressed(KEY_E))                 ed.gridW++;
    }
    if (!IsKeyDown(KEY_LEFT_CONTROL)) {
        if (IsKeyPressed(KEY_W))                 ed.gridH++;
        if (IsKeyPressed(KEY_S) && ed.gridH > 1) ed.gridH--;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 delta = GetMouseDelta();
        ed.camera.target.x -= delta.x / ed.camera.zoom;
        ed.camera.target.y -= delta.y / ed.camera.zoom;
    }

    Vector2 ms = GetMousePosition();
    bool inToolbar = (ms.y < ed.TOOLBAR_H);
    bool inBottom = (ms.y > ed.screenH - ed.BOTTOM_H);
    float wheel = GetMouseWheelMove();

    if (inToolbar && wheel != 0) {
        int cur = (int)ed.currentType;
        if (wheel < 0) cur = (cur + 1) % TC;
        else           cur = (cur - 1 + TC) % TC;
        ed.currentType = (EditorObjectType)cur;
    }
    else if (!inToolbar && wheel != 0) {
        ed.camera.zoom += wheel * 0.1f;
        if (ed.camera.zoom < 0.2f) ed.camera.zoom = 0.2f;
        if (ed.camera.zoom > 3.0f) ed.camera.zoom = 3.0f;
    }
    AdjustToolbarOffset(ed);

    if (inToolbar && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        for (int v = 0; v < ed.VISIBLE_BTNS; v++) {
            int typeIdx = ed.toolbarOffset + v;
            if (typeIdx >= TC) break;
            if (CheckCollisionPointRec(ms, GetBtnRect(ed, v)))
                ed.currentType = (EditorObjectType)typeIdx;
        }
    }

    // ステージ領域（パネル領域外）
    if (!inToolbar && !inBottom && !mouseInPropPanel) {
        Vector2 mw = GetScreenToWorld2D(ms, ed.camera);
        Vector2 sn = SnapToGrid(mw, ed.gridSize);

        // --- 選択中オブジェクトのドラッグ移動 ---
        if (ed.propSelectedIdx >= 0 && ed.propSelectedIdx < (int)ed.objects.size()) {
            auto& sel = ed.objects[ed.propSelectedIdx];

            // 選択オブジェクト上で左クリック開始 -> ドラッグ開始
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mw, sel.rect)) {
                ed.isDragging = true;
                ed.dragStart = { mw.x - sel.rect.x, mw.y - sel.rect.y }; // つかみ位置オフセット
            }

            // ドラッグ中は選択オブジェクトを移動（グリッドスナップ）
            if (ed.isDragging) {
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    Vector2 newPos = { mw.x - ed.dragStart.x, mw.y - ed.dragStart.y };
                    Vector2 snapPos = SnapToGrid(newPos, ed.gridSize);
                    sel.rect.x = snapPos.x;
                    sel.rect.y = snapPos.y;
                } else {
                    ed.isDragging = false;
                }
            }
        }

        // 左クリック: 配置（ドラッグ開始中は配置しない）
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !ed.isDragging) {
            PlacedObject newObj;
            newObj.type = ed.currentType;
            newObj.rect = { sn.x, sn.y, ed.gridSize * ed.gridW, ed.gridSize * ed.gridH };
            InitDefaultParams(newObj);
            ed.objects.push_back(newObj);
        }

        // 右クリック: 削除
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            for (int i = (int)ed.objects.size() - 1; i >= 0; i--) {
                if (CheckCollisionPointRec(mw, ed.objects[i].rect)) {
                    ed.undoStack.push_back(ed.objects[i]);
                    ed.objects.erase(ed.objects.begin() + i);
                    if (ed.propSelectedIdx == i) { ed.propSelectedIdx = -1; ed.propEditingParam = -1; }
                    else if (ed.propSelectedIdx > i) ed.propSelectedIdx--;
                    break;
                }
            }
        }

        // T キー: カーソル下のオブジェクトを選択 → プロパティパネル表示
        if (IsKeyPressed(KEY_T)) {
            bool found = false;
            for (int i = (int)ed.objects.size() - 1; i >= 0; i--) {
                if (CheckCollisionPointRec(mw, ed.objects[i].rect)) {
                    ed.propSelectedIdx = i;
                    ed.propEditingParam = -1;
                    found = true;
                    break;
                }
            }
            if (!found) { ed.propSelectedIdx = -1; ed.propEditingParam = -1; }
        }
    }

    if (ed.propSelectedIdx >= 0 && ed.propSelectedIdx < (int)ed.objects.size()
        && IsKeyPressed(KEY_DELETE)) {
        ed.undoStack.push_back(ed.objects[ed.propSelectedIdx]);
        ed.objects.erase(ed.objects.begin() + ed.propSelectedIdx);
        ed.propSelectedIdx = -1;
        ed.propEditingParam = -1;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z) && !ed.undoStack.empty()) {
        ed.objects.push_back(ed.undoStack.back());
        ed.undoStack.pop_back();
    }
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
        EditorSaveJSON(ed, (ed.savePath + ".json").c_str());
        EditorSaveCSV(ed, (ed.savePath + ".csv").c_str());
        ed.saveNotifyTimer = 2.0f;
    }
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L))
        EditorLoadCSV(ed, (ed.savePath + ".csv").c_str());
}