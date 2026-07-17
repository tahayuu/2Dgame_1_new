#include "StageEditorInternal.h"

// ================================================================
// StageEditor.cpp の役割
// ---------------------------------------------------------------
// ・StageEditor の「編集ロジック本体」を持つ。
// ・入力(キー/マウス)を受けて PlacedObject / PlacedEnemy を更新し、
//   最終的に Export/Save へ渡す元データを作る。
// ・このファイルで決める重要ルール:
//   1) type はギミック挙動の識別子
//   2) spriteId/rotation/flip は見た目専用
//   3) params[p0..p5] は type ごとに意味を切り替える
// ================================================================

// ================================================================
// パラメータテーブル
// ---------------------------------------------------------------
// 目的:
// ・各ギミック(type)に対して、params[p0..p5] の「意味」を定義する。
// ・UIのプロパティ表示名/初期値/bool扱いもここで一元管理する。
//
// 使い方の前提:
// ・PlacedObject は全タイプ共通で params[0..5] を持つ。
// ・どのスロットを使うかは type ごとに異なるため、
//   Export/Import 側はこの定義と必ず対応させる必要がある。
//
// 変更時の注意:
// ・既存スロット順を変えると、保存済みCSVの意味がずれる。
// ・名前変更だけでも、編集UIの表示と説明が変わる。
// ================================================================
static const TypeParamInfo TYPE_PARAMS[(int)EditorObjectType::COUNT] = {//•	新しいオブジェクトを追加しても、COUNT が自動で増える
    /* 0  PLATFORM              */ { 0, {} },
    /* 1  BACK_PLATFORM         */ { 0, {} },
    /* 2  HAZARD                */ { 1, {{"disableSnapGroupId",0,false}} },
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
    /* 24 MOVE_HAZARD_X   */ { 6, {{"raiseW",100,false},{"moveSpd",200,false},{"delay",0,false},{"dir(1or-1)",1,false},{"toleranceX",24.0f,false},{"toleranceY",40.0f,false}} },
    /* 25 MOVE_HAZARD_EXT_X(未使用/互換性維持のため予約) */ { 0, {} },
    /* 26 TRACKING_HAZARD       */ { 4, {{"speed",150,false},{"trkRange",300,false},{"maxDist",400,false},{"retStart",1,true}} },
    /* 27 ROTATING_BALL         */ { 3, {{"radius",15,false},{"angSpd",2,false},{"armLen",100,false}} },
    /* 28 MOVE_ROTATING_BALL    */ { 5, {{"radius",15,false},{"angSpd",2,false},{"gravity",1600,false},{"reverse",0,true},{"armLen",100,false}} },
    /* 29 ROLLING_BALL          */ { 4, {{"radius",15,false},{"rollSpd",200,false},{"rollDir",-1,false},{"waitBtn",1,true}} },
    /* 30 FALLING_PLATFORM      */ { 3, {{"fallDelay",0.5f,false},{"fallSpd",400,false},{"detect(0:both 1:p 2:b)",0,false}} },
    /* 31 UPRISING_PLATFORM     */ { 2, {{"riseDelay",0.3f,false},{"riseSpd",300,false}} },
    /* 32 UP_DOWN_PLATFORM      */ { 4, {{"riseH",150,false},{"riseSpd",200,false},{"fallSpd",200,false},{"stopTm",0.5f,false}} },
    /* 33 CLEAR_BLOCK           */ { 0, {} },
    /* 34 CLEAR_BLOCK_X         */ { 0, {} },
    /* 35 SWITCH_PLATFORM       */ { 4, {{"speedX",100,false},{"speedY",100,false},{"maxX",200,false},{"maxY",200,false}} },
    /* 36 FALLING_TEXT          */ { 4, {{"fallDelay",0.2f,false},{"gravity",1600,false},{"tolerance",300,false},{"fontSize",48,false}} },
    /* 37 EXIT_DOOR             */ { 1, {{"targetStage",1,false}} },
    /* 38 LAYER_DOOR            */ { 0, {} },
    /* 39 RESPAWN               */ { 0, {} },
    /* 40 SWITCH_BUTTON         */ { 0, {} },
    /* 41 COMMENT_BLOCK         */ { 1, {{"duration",3.0f,false}} },
    /* 42 CURSOR_BOTTOM         */ { 3, {{"targetBall",-1,false},{"oneShot",1,true},{"maxDist",0,false}} },
    /* 43 DEATH_BLOCK           */ { 0, {} },
    /* 44 SPIKE_BOUNCER         */ { 2, {{"bounceVelX",0,false},{"bounceVelY",-600,false}} },
    /* 45 SPRING                */ { 2, {{"bounceVelX",0,false},{"bounceVelY",-800,false}} },
    /* 46 CRANE_LAUNCH_PAD      */ { 1, {{"launchVelY",-900,false}} },
    /* 47 CRANE                 */ { 5, {{"maxArmLen",200,false},{"detectRangeY",600,false},{"carrySpeedX",250,false},{"carryDir",1,false},{"carryDist",2000,false}}},
    /* 48 OJISAN_PUNCH_AREA     */ { 0, {} },
    /* 49 WARP_HOLE             */ { 2, {{"destX",0,false},{"destY",0,false}} },
    /*50  ENEMY                 */ {0,{} },
    /* 51 TEMP_FLOOR            */ { 1, {{"showSec",2.0f,false}} },
    /* 52 TEMP_FLOOR_SWITCH     */ { 2, {{"targetFloor",-1,false},{"oneShot",1,true}} },
    /* 53 DECOR_ARROW           */ { 1, {{"angleDeg",0.0f,false}} },
    /* 54 DECOR_SPRITE          */ {2,{{"eventId",0,false},{"drawFront",0,true}}},
    /* 55 MOVEPLATFORMYXY        */ { 5, {{"moveSpeedY1",200,false},{"moveDistanceY1",200,false},{"moveDistanceX2",200,false},{"moveDistanceY3",200,false},{"delay",0,false}} },
    /* 56 EVENT_CHANGER */{5,{{"targetEventId", 1, false},{"changedSpriteId", 0, false},{"jumpMode(0:normal 1:N)", 1, false},{"restoreOnExit", 1, true},{"oneShot", 0, true}} },
    /* 57 DRAG_PIECE */ {6,{{"groupId",1,false},{"pieceId",1,false},{"startSlotId",-1,false},{"returnOnMiss",1,true},{"allowSwap",1,true},{"lockOnSolve",1,true}} },
    /* 58 SNAP_SLOT */ {6,{{"groupId",1,false},{"slotId",1,false},{"requiredPieceId",1,false},{"snapRadius",60,false},{"showGuide",1,true},{"lockOnCorrect",0,true}} },
};




static const EnemyTypeParamInfo ENEMY_TYPE_PARAMS[] = {
    // [0] WALKER
    {  2,  // パラメータ数2
        {{"Speed (移動速度)", 100.0f, false},{"PatrolDist (巡回距離)", 200.0f, false}  } },
    // [1] FLYER
    {  2,  // パラメータ数2
    {{"Speed (移動速度)", 80.0f, false},         
         {"HoverHeight (滞空高さ)", 100.0f, false}    
        }
    },
    // [2] SHOOTER
    {3,  // パラメータ数3
        {{"Speed (移動速度)", 60.0f, false},         
         {"ShootInterval (発射間隔)", 2.0f, true},   
         {"ShootRange (射撃範囲)", 300.0f, false}    
        }
    },
    // [3] JUMPCOPY
    {  2,  // パラメータ数2
    {{"Speed (移動速度)", 120.0f, false},      
      {"JumpHeight (ジャンプ高さ)", 150.0f, false} 
        }
    },
};

// 敵のタイプに応じたパラメータ情報を取得する関数
const EnemyTypeParamInfo& EdGetEnemyTypeInfo(EnemyType t) {//敵が持つべきパラメータ情報を取得する
    int idx = static_cast<int>(t);//enumをintに変換
	if (idx < 0 || idx >= 4) return ENEMY_TYPE_PARAMS[0];//範囲外の場合はデフォルトの WALKER のパラメータ情報を返す
    return ENEMY_TYPE_PARAMS[idx];
}

//デフォルトパラメータを初期化
void InitDefaultEnemyParams(PlacedEnemy& enemy) {
	const auto& info = EdGetEnemyTypeInfo(enemy.type);//敵のタイプに応じたパラメータ情報を取得
    for (int i = 0; i < MAX_OBJ_PARAMS; i++) {
		enemy.params[i] = (i < info.count) ? info.defs[i].defaultValue : 0.0f;//パラメータ数を超えた場合は0.0fで初期化
    }
}

int GetEnemyAtWorldPos(const StageEditor& ed, Vector2 worldPos) {//敵の配置位置を取得する
    for (int i = (int)ed.placedEnemies.size() - 1; i >= 0; i--)
        if (CheckCollisionPointRec(worldPos, GetEnemyRect(ed.placedEnemies[i])))
			return i;//敵の矩形にワールド座標が含まれている場合は、その敵のインデックスを返す
    return -1;
}

Rectangle GetEnemyRect(const PlacedEnemy& enemy) {
    const float R = 20.0f;
	return { enemy.pos.x - R,enemy.pos.y - R,R * 2.0f,R * 2.0f };//敵の矩形を返す（中心座標から半径20の正方形）
}

void EditorAddEnemy(StageEditor& ed, EnemyType type, Vector2 worldPos) {
    PlacedEnemy e;
    e.type = type;
    e.pos = worldPos;
    InitDefaultEnemyParams(e);
    ed.placedEnemies.push_back(e);
}

void EditorRemoveEnemy(StageEditor& ed, int idx) {//敵を削除する
    if (idx < 0 || idx >= (int)ed.placedEnemies.size()) return;
    ed.placedEnemies.erase(ed.placedEnemies.begin() + idx);
	if //選択中の敵インデックスが削除された敵のインデックスと同じ場合は、選択を解除する
        (ed.selectedEnemyIdx == idx) ed.selectedEnemyIdx = -1;//選択中の敵が削除された場合は選択を解除
	else if//選択中の敵インデックスが削除された敵のインデックスより大きい場合は、インデックスを1つ減らす
        (ed.selectedEnemyIdx > idx) ed.selectedEnemyIdx--;//選択中の敵インデックスを更新
    /*具体例

削除前が次の状態だとします。

placedEnemies[0] 歩行敵
placedEnemies[1] 飛行敵
placedEnemies[2] 砲台敵
placedEnemies[3] ジャンプ敵

現在、ジャンプ敵を選択しています。

ed.selectedEnemyIdx = 3;

ここで、飛行敵を削除します。

idx = 1;

削除後は次の状態です。

placedEnemies[0] 歩行敵
placedEnemies[1] 砲台敵
placedEnemies[2] ジャンプ敵

ジャンプ敵の番号は、3から2へ変わりました。*/
}

void EditorSetEnemyParam(StageEditor& ed, int enemyIdx, int paramIdx, float value) {//敵のパラメータを設定する
	if (enemyIdx >= 0 && enemyIdx < (int)ed.placedEnemies.size()
        && paramIdx >= 0 && paramIdx < MAX_OBJ_PARAMS) {//範囲チェック
		ed.placedEnemies[enemyIdx].params[paramIdx] = value;//敵のパラメータを設定
    }
}

const TypeParamInfo& EdGetTypeInfo(EditorObjectType t) {//EditorObjectType は enum class なので int にキャストして配列アクセス
	return TYPE_PARAMS[(int)t];//enum class EditorObjectType の値が増えても、TYPE_PARAMS 配列のサイズは自動で増える
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
    // ★ Sprite(見た目)情報の表示欄（区切り線 + 3行分）を追加する
    //   DrawPropertyPanel 側の追加描画量と対応させてあるので、
    //   両方まとめて変更する場合はここも忘れないこと。
    h += PROP_LINE_H * 3.0f + 10.0f;
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
    "movePlatformsX","moveHazards","moveHazardsExtY","moveDownHazardsExtY","moveHazardX",
    "unused_moveHazardsExtX","trackingHazards","rotatingBalls","moveRotatingBalls","rollingBalls",
    "fallingPlatforms","upRisingPlatforms","upDouwnPlatforms","clearBlocks","clearBlocksX",
    "switchPlatforms","fallingTexts","exitDoors","layerDoors","respawn","switchButtons",
    "commentBlocks","cursorButtons","deathBlocks","spikeBouncers","springs",
	"craneLaunchPads","cranes","ojisanPunchAreas","warpHoles","enemies","tempFloors","tempFloorSwitches","decorArrows",
    "decorSprites","movePlatformYXY","eventChangers","dragPieces","snapSlots"
};

static const char8_t* const JP_NAMES_U8[(int)EditorObjectType::COUNT] = {
  u8"床",u8"背景床",u8"トゲ",u8"触れると壊れるブロック",u8"下から叩くと壊れるブロック",u8"壊れるブロック",
  u8"エレベーター",u8"重力ブロック",u8"ボタンブロック",u8"アイテムブロック",u8"氷床",u8"下降床",u8"上昇床",
  u8"ジャンプ台",u8"バッテリー人間",u8"カーソル床",u8"磁石",u8"ノックバック壁",u8"分裂床",u8"円移動床",u8"左右移動床",
  u8"左右移動トゲ",u8"上下移動トゲ",u8"下降トゲ",u8"左右移動トゲ",u8"未使用トゲ",u8"追尾トゲ",u8"回転ボール",
  u8"移動回転ボール",u8"転がるボール",u8"落下床",u8"上昇床",u8"上下移動床",u8"クリアブロック",u8"横クリアブロック",
  u8"スイッチ床",u8"落下文字",u8"ゴール",u8"レイヤードア",u8"リスポーン",u8"スイッチ",u8"コメント",u8"カーソルスイッチ",
  u8"即死ブロック",u8"トゲジャンプ",u8"ばね",u8"クレーン発射台",u8"クレーン",u8"おじさんパンチ",u8"ワープ",u8"敵",u8"一時床",
  u8"一時床スイッチ",u8"矢印",u8"装飾",u8"Ｙ→Ｘ→Ｙ移動床",u8"イベント変更エリア",u8"ドラッグピース",u8"スナップスロット"
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
    {180, 50, 50,255},      // 24 MoveHazardRightX(dir対応)
    {80, 80, 80, 100},      // 25 未使用（旧MoveHazardExtX）
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
		{220, 140,  60, 255},   // 40 SwitchButton
  { 60, 200, 200, 255},   // 41 CommentBlock
	{255,200,0,255},	  // 42 CursorBottom
	{255, 40, 140, 255},      // 43 DeathBlock
	   {200, 80, 200, 255},      // 44 SpikeBouncer
		   {255, 192, 203, 255},     // 45 Spring
	{255, 220,   0, 255},     // 46 CraneLaunchPad
	{120, 120, 120, 255},     // 47 Crane
	{255, 120, 120, 255},   // 48 OjisanPunchArea
	{100, 200, 255, 255},   // 49 WarpHole
	{200, 100, 100, 255},      // 50 Enemy
    {120,220,255,255},   // 51 TEMP_FLOOR
   {255,180, 60,255},   // 52 TEMP_FLOOR_SWITCH
   {100, 255, 180, 255},   // 53 DECOR_ARROW
	{255, 255, 100, 200 },   // 54 MOVEPLATFORMYXY{}
      {255, 255, 100, 200},   // 55 DECO_SPRITE
      {255, 100, 255, 120}, // 56 EVENT_CHANGER
      {100,220,255,220},  // 57 DRAG_PIECE
{255,220,100,160},  // 58 SNAP_SLOT
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
        DrawRectangle((int)(cx - 6), (int)(r.y + r.height - 6), 12, 6, { 220,140,60,255 });
        DrawCircle((int)cx, (int)(r.y + r.height / 2 - 2), 7, { 220,140,60,255 });
        DrawCircleLines((int)cx, (int)(r.y + r.height / 2 - 2), 7, BLACK);
    }
    else if (typeIdx == 42) { // CURSOR_BOTTOM 追加
        DrawRectangleRec(r, ColorAlpha(c, 0.85f));
        DrawRectangleLinesEx(r, 2, DARKBROWN);
        float cx = r.x + r.width * 0.5f;
        float cy = r.y + r.height * 0.5f;
        DrawCircle((int)cx, (int)cy, 4.0f, BLACK);
    }
    else if (typeIdx == 49) { // WARP_HOLE: 渦巻き形状
        float cx = r.x + r.width / 2;
        float cy = r.y + r.height / 2;
        float rad = (std::min)(r.width, r.height) / 2 - 2;
        DrawCircle((int)cx, (int)cy, rad, c);
        DrawCircleLines((int)cx, (int)cy, (int)(rad * 0.7f), BLACK);
        DrawCircleLines((int)cx, (int)cy, (int)(rad * 0.4f), BLACK);
    }
    else if (typeIdx == (int)EditorObjectType::OJISAN_PUNCH_AREA) {
        DrawRectangleRec(r, ColorAlpha(c, 0.8f));
        DrawRectangleLinesEx(r, 2, MAROON);
        DrawText("!", (int)(r.x + r.width / 2 - 3), (int)(r.y + 2), 14, BLACK);
    }
    else if (typeIdx == 50) {
        float cx = r.x + r.width / 2;
        float cy = r.y + r.height / 2;
        float rad = (std::min)(r.width, r.height) / 2 - 2;
        DrawCircle((int)cx, (int)cy, rad, c);
        DrawCircleLines((int)cx, (int)cy, rad, BLACK);
        DrawText("E", (int)(cx - 6), (int)(cy - 8), 16, BLACK);
    }

    else if (typeIdx == (int)EditorObjectType::DECOR_SPRITE) {
        // 絵のフレームっぽいアイコン（当たり判定なしを示すため薄い色）
        DrawRectangleRec(r, ColorAlpha({255,255,100,255}, 0.3f));
        DrawRectangleLinesEx(r, 2, {200, 180, 0, 200});
        float cx = r.x + r.width * 0.5f;
        float cy = r.y + r.height * 0.5f;
        // 対角線でスプライト感を表現
        DrawLineEx({r.x + 3, r.y + 3}, {r.x + r.width - 3, r.y + r.height - 3}, 1.5f, {200,180,0,200});
        DrawLineEx({r.x + r.width - 3, r.y + 3}, {r.x + 3, r.y + r.height - 3}, 1.5f, {200,180,0,200});
    }
    else {
        DrawRectangleRec(r, c);
        DrawRectangleLinesEx(r, 1, ColorAlpha(BLACK, 0.4f));
    }
}

// ================================================================
// 目的: エディタ編集を開始できる初期状態へ揃える。
// 入力: 画面サイズ・UIフォント。
// 出力: ed の内部状態を初期化し、既存CSVがあれば復元する。
// 注意: savePath を変更している場合、ここで自動読込される対象も変わる。
void EditorInit(StageEditor& ed, int screenWidth, int screenHeight, Font uiFont) {
    ed.active = true;
    ed.objects.clear();
    ed.undoStack.clear();
    ed.currentType = EditorObjectType::PLATFORM;
    ed.placedEnemies.clear();
    ed.selectedEnemyIdx = -1;
    ed.currentEnemyType = EnemyType::WALKER;
    ed.hoveredEnemyType = -1;
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
// 目的: エディタ1フレーム分の編集入力を処理する中核関数。
// 入力: 現在の入力状態と経過時間 dt。
// 出力: objects / placedEnemies / 選択状態 / 保存状態が更新される。
// 注意:
// ・この関数の結果が SaveCSV/ExportToStage の元データになる。
// ・入力処理は「編集中モード(return) → 通常配置操作」の順で分岐しているため、
//   分岐順を変えると誤入力や同時操作バグが起きやすい。
void EditorUpdate(StageEditor& ed, float dt) {
    if (!ed.active) return;
	if (ed.saveNotifyTimer > 0.0f) ed.saveNotifyTimer -= dt;//保存通知タイマーを減算

	if (ed.propSelectedIdx >= (int)ed.objects.size()) {//選択中のオブジェクトが削除されている場合は、選択を解除する
		ed.propSelectedIdx = -1;//選択中のオブジェクトが削除されている場合は、選択を解除する
		ed.propEditingParam = -1;//選択中のオブジェクトが削除されている場合は、パラメータ編集を終了する
    }

	const int TC = (int)EditorObjectType::COUNT;//EditorObjectType の数を取得

	bool mouseInPropPanel = false;//マウスがプロパティパネル内にあるかどうかを判定するフラグ
	if (ed.propSelectedIdx >= 0)//選択中のオブジェクトがある場合は、プロパティパネルの矩形を取得して、マウスがその中にあるかどうかを判定する
		mouseInPropPanel = CheckCollisionPointRec(GetMousePosition(), GetPropPanelRect(ed));//マウスがプロパティパネル内にあるかどうかを判定する


    // === 敵パラメータ テキスト編集中 ===
    if (ed.currentType == EditorObjectType::ENEMY &&
        ed.selectedEnemyIdx >= 0 && ed.propEditingParam >= 0) {

        // キーボードから入力された文字を1文字取得する
        int ch = GetCharPressed();

        // 入力された文字が残っている間、すべて処理する
        while (ch > 0) {

            // 数字、小数点、マイナスだけ入力を許可する
            // これにより、3.5 や -2.0 のような数値を入力できる
            if ((ch >= '0' && ch <= '9') || ch == '.' || ch == '-') {

                // 入力文字数が30文字未満なら追加する
                // 長すぎる入力を防ぐため
                if (ed.propEditCursor < 30) {

                    // 現在のカーソル位置に文字を入れる
                    ed.propEditBuf[ed.propEditCursor++] = (char)ch;

                    // C言語文字列の終端を入れる
                    ed.propEditBuf[ed.propEditCursor] = '\0';
                }
            }

            // 次に入力された文字を取得する
            ch = GetCharPressed();
        }

        // Backspaceが押されたら、入力中の文字を1文字削除する
        if (IsKeyPressed(KEY_BACKSPACE) && ed.propEditCursor > 0) {
            ed.propEditBuf[--ed.propEditCursor] = '\0';
        }

        // Enterが押されたら、入力した文字列を数値に変換して反映する
        if (IsKeyPressed(KEY_ENTER)) {

            // propEditBufに入っている文字列をfloatに変換して、
            // 選択中の敵のパラメータに設定する
            EditorSetEnemyParam(ed,
                ed.selectedEnemyIdx,
                ed.propEditingParam,
                (float)atof(ed.propEditBuf));

            // 編集終了
            ed.propEditingParam = -1;
        }

        // Escapeが押されたら、変更せずに編集をやめる
        if (IsKeyPressed(KEY_ESCAPE)) {
            ed.propEditingParam = -1;
        }

        // テキスト編集中は、他のエディタ操作をさせない
        return;
    }




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
                // EXIT_DOOR の targetStage パラメータの特別処理
                if (ed.objects[ed.propSelectedIdx].type == EditorObjectType::EXIT_DOOR &&
                    strcmp(info.defs[i].name, "targetStage") == 0) {
                    // ドロップダウンクリック処理（次のステージを選択）
                    float& val = ed.objects[ed.propSelectedIdx].params[i];
                    int stageIdx = (int)val;
                    stageIdx = (stageIdx + 1) % 6;  // 0～5 でループ
                    val = (float)stageIdx;
                } 
                // WARP_HOLE の destX, destY パラメータ編集
                else if (ed.objects[ed.propSelectedIdx].type == EditorObjectType::WARP_HOLE &&
                         (strcmp(info.defs[i].name, "destX") == 0 || strcmp(info.defs[i].name, "destY") == 0)) {
                    ed.propEditingParam = i;
                    snprintf(ed.propEditBuf, sizeof(ed.propEditBuf), "%.0f",
                        ed.objects[ed.propSelectedIdx].params[i]);
                    ed.propEditCursor = (int)strlen(ed.propEditBuf);
                }
                else if (info.defs[i].isBool) {
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

    // ================================================================
  // === Sprite(見た目) 変更操作 ===
  // ----------------------------------------------------------------
  // 選択中オブジェクト(ed.propSelectedIdx)に対して、
  // 「見た目(SpriteId)」だけを変更するキー操作。
  // ここで変更するのは spriteId / rotation / flipX / flipY だけで、
  // obj.type（ギミックの種類）や obj.rect（当たり判定）は一切変更しない。
  // ※ この位置に置くことで、数値パラメータ編集中やテキスト編集中
  //   （このコードより前で return 済み）には反応しないようにしている。
  // ================================================================
    if (ed.propSelectedIdx >= 0 && ed.propSelectedIdx < (int)ed.objects.size()) {
        auto& sel = ed.objects[ed.propSelectedIdx];
        const int spriteCount = (int)SpriteId::COUNT;

        // ] キー: 次のSpriteIdへ切り替え
        if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
            int next = ((int)sel.spriteId + 1) % spriteCount;
            sel.spriteId = (SpriteId)next;
        }
        // [ キー: 前のSpriteIdへ切り替え
        if (IsKeyPressed(KEY_LEFT_BRACKET)) {
            int next = ((int)sel.spriteId - 1 + spriteCount) % spriteCount;
            sel.spriteId = (SpriteId)next;
        }

        // H キー: 左右反転を切り替え
        if (IsKeyPressed(KEY_H)) sel.flipX = !sel.flipX;
        // J キー: 上下反転を切り替え
        if (IsKeyPressed(KEY_J)) sel.flipY = !sel.flipY;

        // K キー: 反時計回りに90度回転 / L キー: 時計回りに90度回転
        if (IsKeyPressed(KEY_K)) sel.rotation -= 90.0f;
        if (IsKeyPressed(KEY_L)) sel.rotation += 90.0f;

        // 角度を 0〜360 の範囲に収める（表示を見やすくするための処理）
        if (sel.rotation >= 360.0f) sel.rotation -= 360.0f;
        if (sel.rotation < 0.0f)    sel.rotation += 360.0f;

        // ================================================================
        // === コピー・ペースト機能 ===
        // ================================================================
        // C キー: 選択中のオブジェクトをコピー
        if (IsKeyPressed(KEY_C)) {
            ed.clipboard = sel;
            ed.hasClipboard = true;
        }

        // P キー: クリップボードの内容を貼り付け（新しいオブジェクトとして追加）
        if (IsKeyPressed(KEY_P) && ed.hasClipboard) {
            // クリップボードの内容を新しいオブジェクトとしてコピー
            PlacedObject newObj = ed.clipboard;
            // 位置をずらして配置（重ならないようにする）
            newObj.rect.x += 30;
            newObj.rect.y += 30;
            ed.objects.push_back(newObj);
        }

        // ================================================================
        // === サイズ変更機能 ===
        // ================================================================
        // マイナスキー（-）: 幅・高さを減らす
        if (IsKeyPressed(KEY_MINUS)) {
            sel.rect.width = std::max(10.0f, sel.rect.width - 10.0f);
            sel.rect.height = std::max(10.0f, sel.rect.height - 10.0f);
        }
        // プラスキー（=）: 幅・高さを増やす
        if (IsKeyPressed(KEY_EQUAL)) {
            sel.rect.width += 10.0f;
            sel.rect.height += 10.0f;
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
        // ------------------------------------------------------------
        // 配置フローの要点:
        // 1) 画面座標→ワールド座標へ変換
        // 2) SnapToGrid で配置基準をそろえる
        // 3) type に応じて PlacedObject / PlacedEnemy を追加
        //
        // ここで作った配列データが、後で ExportToStage により
        // 実行時Stageデータへ変換される。
  //  マウスが敵選択サイドパネル上か調べる
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !ed.isDragging) {
            if (ed.currentType == EditorObjectType::ENEMY) {
				Rectangle enemySidePanel = { 10.0f,ed.TOOLBAR_H + 10.0F,130.0F,175.0F };// サイドパネルの矩形を定義
				bool onSidePanel = CheckCollisionPointRec(GetMousePosition(), enemySidePanel);// サイドパネルのチェック

                //  プロパティパネル上かどうかをfalseで初期化する
                bool onPropPanel = false;
                //一つ目の条件selectedEnemyIdxが0以上なら何らかの敵が選択されている
				//二つ目の条件selectedEnemyIdxがplacedEnemiesのサイズより小さいなら、選択されている敵のインデックスが有効である
                if (ed.selectedEnemyIdx >= 0 && ed.selectedEnemyIdx < (int)ed.placedEnemies.size()) {
					const auto& info = EdGetEnemyTypeInfo(ed.placedEnemies[ed.selectedEnemyIdx].type);// 敵タイプ情報を取得
                    float panelW = 280.0f;  // PROP_W
                    float panelH = 58.0f + (float)info.count * 28.0f + 40.0f;  // PROP_HEADER_H + count * PROP_LINE_H + 40
                    float panelX = ed.screenW - panelW - 10.0f;
                    float panelY = ed.TOOLBAR_H + 10.0f;
                    Rectangle enemyPropPanel = { panelX, panelY, panelW, panelH };
					onPropPanel = CheckCollisionPointRec(GetMousePosition(), enemyPropPanel);// 敵プロパティパネルのチェック
                }
				// サイドパネル上でもプロパティパネル上でもない場合に敵を配置する
                if (!onSidePanel && !onPropPanel) {
                    EditorAddEnemy(ed, ed.currentEnemyType, sn);
					ed.selectedEnemyIdx = (int)ed.placedEnemies.size() - 1;// 追加した敵を選択状態にする
                    /*例えば、敵を追加した後の要素数が4なら、ed.placedEnemies.size() == 4 配列番号は次のようになります。
                             0
                             1
                             2
                             3                       最後の要素番号は、4 - 1 = 3です。*/
                }
            }
			// それ以外のオブジェクトタイプの場合は、PlacedObject を追加
            else {
				PlacedObject newObj;// 新しいオブジェクトを作成
				newObj.type = ed.currentType;// オブジェクトの種類を設定
				newObj.rect = { sn.x, sn.y, ed.gridSize * ed.gridW, ed.gridSize * ed.gridH };// オブジェクトの位置とサイズを設定
				InitDefaultParams(newObj);// デフォルトパラメータを初期化
                ed.objects.push_back(newObj);
            }
        }

        // 右クリック: 削除
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            if (ed.currentType == EditorObjectType::ENEMY) {
                int hitIdx = GetEnemyAtWorldPos(ed, mw);
                if (hitIdx >= 0) EditorRemoveEnemy(ed, hitIdx);
            }else{
            for (int i = (int)ed.objects.size() - 1; i >= 0; i--) {
                if (CheckCollisionPointRec(mw, ed.objects[i].rect)) {
					ed.undoStack.push_back(ed.objects[i]);// 削除前の状態を undoStack に保存
					ed.objects.erase(ed.objects.begin() + i);// オブジェクトを削除
					if (ed.propSelectedIdx == i) { ed.propSelectedIdx = -1; ed.propEditingParam = -1; }// 選択中のオブジェクトが削除された場合、選択状態を解除
					else if (ed.propSelectedIdx > i) ed.propSelectedIdx--;// 選択中のオブジェクトのインデックスを調整
                    break;
                }
                }
            }
        }

        // T キー: カーソル下のオブジェクトを選択 → プロパティパネル表示
        if (IsKeyPressed(KEY_T)) {
            if (ed.currentType == EditorObjectType::ENEMY) {
				ed.selectedEnemyIdx = GetEnemyAtWorldPos(ed, mw);// カーソル下の敵を選択
				ed.propEditingParam = -1;// 選択中の敵のパラメータ編集を終了
            }else{
				bool found = false;// カーソル下のオブジェクトが見つかったかどうかを示すフラグ
			for (int i = (int)ed.objects.size() - 1; i >= 0; i--) {// 配列の後ろから前に向かって検索（上に重なっているオブジェクトを優先）
				if (CheckCollisionPointRec(mw, ed.objects[i].rect)) {// カーソル下のオブジェクトを見つけた場合
					ed.propSelectedIdx = i;// カーソル下のオブジェクトを選択
					ed.propEditingParam = -1;// 選択中のオブジェクトのパラメータ編集を終了
					found = true;// オブジェクトが見つかったことを示すフラグを設定
                    break;//探索を終了
                }
                }
			if (!found) { ed.propSelectedIdx = -1; ed.propEditingParam = -1; }// カーソル下にオブジェクトが見つからなかった場合、選択状態を解除
            }
            
        }
    }

    if (ed.currentType == EditorObjectType::ENEMY && ed.selectedEnemyIdx >= 0 && IsKeyPressed(KEY_DELETE)) {
        EditorRemoveEnemy(ed, ed.selectedEnemyIdx);
    }

    if (ed.propSelectedIdx >= 0 && ed.propSelectedIdx < (int)ed.objects.size()
        && IsKeyPressed(KEY_DELETE)) {
        ed.undoStack.push_back(ed.objects[ed.propSelectedIdx]);
        ed.objects.erase(ed.objects.begin() + ed.propSelectedIdx);
        ed.propSelectedIdx = -1;
        ed.propEditingParam = -1;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z) && !ed.undoStack.empty()) {
        /*undoStackの最後に保存されているオブジェクトを、通常オブジェクト一覧の末尾へ追加*/
        ed.objects.push_back(ed.undoStack.back());
		//undoStackの最後の要素を削除することで、次回のCtrl+Zで同じオブジェクトが再度復元されないようにする
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