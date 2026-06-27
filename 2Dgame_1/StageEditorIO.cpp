#include "StageEditorInternal.h"
#include "DialogManager.h" 
#include "EnemyManager.h"
#include <fstream>
#include <sstream>

// ================================================================
//  Export (editor -> Stage)
// ================================================================
void EditorExportToStage(const StageEditor& ed, Stage& stage,EnemyManager& enemyManager) {
    StageClear(stage);
    int c[(int)EditorObjectType::COUNT] = {};

    // SWITCH_BUTTON を配置順に収集 → SWITCH_PLATFORM と 1:1 でペアリング
    std::vector<Rectangle> switchBtnRects;
    int switchPlatformTotal = 0;
    for (const auto& o : ed.objects) {
        if (o.type == EditorObjectType::SWITCH_BUTTON) switchBtnRects.push_back(o.rect);
        if (o.type == EditorObjectType::SWITCH_PLATFORM) switchPlatformTotal++;
    }
    int switchBtnIdx = 0;
    int switchBtnSeen = 0;

    for (const auto& o : ed.objects) {
        int t = (int)o.type;
        switch (o.type) {
        case EditorObjectType::PLATFORM:
            if (c[t] < MAX_PLATFORMS) stage.platforms[c[t]++] = o.rect; break;
        case EditorObjectType::BACK_PLATFORM:
            if (c[t] < MAX_PLATFORMS) stage.backPlatforms[c[t]++] = o.rect; break;
        case EditorObjectType::HAZARD:
            if (c[t] < MAX_HAZARDS) stage.hazards[c[t]++] = o.rect; break;
        case EditorObjectType::TOUCH_BREAK_BLOCK:
            if (c[t] < MAX_PLATFORMS) {
                int i = c[t]++; stage.touchBreakBlocks[i].rect = o.rect;
                stage.touchBreakBlocksInit[i] = stage.touchBreakBlocks[i];
            }break;
        case EditorObjectType::BOTTOM_BREAK_BLOCK:
            if (c[t] < MAX_HAZARDS) {
                int i = c[t]++; stage.bottomBreakBlocks[i].rect = o.rect;
                stage.bottomBreakBlocksInit[i] = stage.bottomBreakBlocks[i];
            }break;
        case EditorObjectType::BREAKABLE_BLOCK:
            if (c[t] < MAX_HAZARDS) {
                int i = c[t]++; stage.breakableBlocks[i].rect = o.rect;
                stage.breakableBlocks[i].isBroken = false;
                stage.breakableBlocksInit[i] = stage.breakableBlocks[i];
            }break;
        case EditorObjectType::GRAVITY_BLOCK:
            if (c[t] < MAX_PLATFORMS) {
                int i = c[t]++; stage.gravityBlocks[i].rect = o.rect;
                stage.gravityBlocksInit[i] = stage.gravityBlocks[i];
            }break;
        case EditorObjectType::ITEM_BLOCK:
            if (c[t] < MAX_PLATFORMS) {
                int i = c[t]++; stage.itemBlocks[i].rect = o.rect;
                stage.itemBlocksInit[i] = stage.itemBlocks[i];
            }break;
        case EditorObjectType::ICE_PLATFORM:
            if (c[t] < MAX_PLATFORMS) {
                int i = c[t]++; auto& p = stage.icePlatforms[i];
                p.rect = o.rect; p.friction = o.params[0]; p.iceAccel = o.params[1]; p.maxSlideSpeed = o.params[2];
                stage.iceplatformsInit[i] = p;
            }break;
        case EditorObjectType::MOVE_DOWN_PLATFORM:
            if (c[t] < MAX_PLATFORMS) {
                int i = c[t]++; stage.moveDownPlatforms[i].rect = o.rect;
                stage.moveDownplatformsInit[i] = stage.moveDownPlatforms[i];
            }break;
        case EditorObjectType::MOVE_UP_PLATFORM:
            if (c[t] < MAX_PLATFORMS) {
                int i = c[t]++; stage.moveUpPlatforms[i].rect = o.rect;
                stage.moveUpplatformsInit[i] = stage.moveUpPlatforms[i];
            }break;
        case EditorObjectType::CURSOR_PLATFORM:
            if (c[t] < MAX_PLATFORMS) {
                int i = c[t]++; auto& p = stage.cursorPlatforms[i];
                p.rect = o.rect; p.followSpeed = o.params[0]; p.maxDistance = o.params[1]; p.isActive = true;
                stage.cursorplatformsInit[i] = p;
            }break;
        case EditorObjectType::MAGNET:
            if (c[t] < MAX_PLATFORMS) {
                int i = c[t]++; auto& m = stage.magnets[i];
                m.rect = o.rect; m.Force = o.params[0]; m.maxDistance = o.params[1]; m.isActive = true;
                stage.magnetsInit[i] = m;
            }break;
        case EditorObjectType::CLEAR_BLOCK:
            if (c[t] < MAX_CLEAR) {
                int i = c[t]++; stage.clearBlocks[i].rect = o.rect;
                stage.clearBlocks[i].clearflag = false;
                stage.clearBlocksInit[i] = stage.clearBlocks[i];
            }break;
        case EditorObjectType::CLEAR_BLOCK_X:
            if (c[t] < MAX_CLEAR) {
                int i = c[t]++; stage.clearBlocksX[i].rect = o.rect;
                stage.clearBlocksX[i].clearflag = false;
                stage.clearBlocksXInit[i] = stage.clearBlocksX[i];
            }break;
        case EditorObjectType::ELEVATOR:
            if (c[t] < Stage::MAX_ELEVATORS) {
                int i = c[t]++; auto& e = stage.elevators[i];
                e.rect = o.rect;
                e.speed = o.params[0];
                e.upperY = o.rect.y - o.params[1]; // rangeUp: 初期位置から何px上まで
                e.lowerY = o.rect.y + o.params[2]; // rangeDown: 初期位置から何px下まで
                stage.elevatorsInit[i] = e;
            }break;
        case EditorObjectType::JUMP_PLATFORM:
            if (c[t] < MAX_PLATFORMS) {
                int i = c[t]++; auto& j = stage.jumpPlatfroms[i];
                j.rect = o.rect; j.jumpVelocity = { o.params[0],o.params[1] }; j.startY = o.rect.y;
                j.stopTimer = o.params[2]; j.delay = o.params[3];
                j.withdraw = o.params[4]; j.jumpSpeed = o.params[5];
                stage.jumpPlatfromsInit[i] = j;
            }break;
        case EditorObjectType::BATTERY_HUMAN:
            if (c[t] < MAX_PLATFORMS) {
                int i = c[t]++; auto& b = stage.batteryHumans[i];
                b.rect = o.rect; b.BatteryVelocity = { o.params[0],o.params[1] };
                b.delay = o.params[2];
                stage.batteryHumansInit[i] = b;
            }break;
        case EditorObjectType::KNOCKBACK_WALL:
            if (c[t] < MAX_PLATFORMS) {
                int i = c[t]++; auto& w = stage.knockBackWalls[i];
                w.rect = o.rect; w.startX = o.rect.x; w.knockBackVelocity = { o.params[0],o.params[1] };
                w.withdraw = o.params[2]; w.delay = o.params[3]; w.limitStop = o.params[4];
                stage.knockBackWallsInit[i] = w;
            }break;
        case EditorObjectType::SPLIT_PLATFORM:
            if (c[t] < MAX_SPLITPLATFORM) {
                int i = c[t]++; auto& s = stage.splitPlatforms[i];
                s.base = o.rect; s.baseInit = o.rect; s.triggered = false; s.offsetX = 0;
                s.maxGap = o.params[0]; s.splitSpeed = o.params[1];
                s.left = { o.rect.x,o.rect.y,o.rect.width / 2,o.rect.height };
                s.right = { o.rect.x + o.rect.width / 2,o.rect.y,o.rect.width / 2,o.rect.height };
                stage.splitplatformsInit[i] = s;
            }break;
        case EditorObjectType::CIRCLE_PLATFORM:
            if (c[t] < MAX_PLATFORMS) {
                int i = c[t]++; auto& cp = stage.circlePlatforms[i];
                cp.center = { o.rect.x + o.rect.width / 2,o.rect.y + o.rect.height / 2 };
                cp.radius = o.params[0]; cp.angle = 0; cp.angleSpeed = o.params[1]; cp.armLength = o.rect.width / 2;
                stage.circleplatformsInit[i] = cp;
            }break;
        case EditorObjectType::MOVE_PLATFORM_X:
            if (c[t] < MAX_MOVEPLATFORM) {
                int i = c[t]++; auto& m = stage.movePlatformsX[i];
                m.rect = o.rect; m.moveSpeed = o.params[0]; m.moveDistance = o.params[1];
                m.timer = 0; m.delay = o.params[2]; m.triggerd = false; m.ismoved = false; m.onplayer = false;
                stage.moveplatformsXInit[i] = m;
            }break;
        case EditorObjectType::MOVE_HAZARD:
            if (c[t] < MAX_MOVEHAZARD) {
                int i = c[t]++; auto& h = stage.moveHazards[i];
                h.rect = o.rect; h.ismoved = false; h.triggerd = false;
                h.raiseHeight = o.params[0]; h.startY = o.rect.y; h.moveSpeed = o.params[1];
                stage.moveHazardsInit[i] = h;
            }break;
        case EditorObjectType::MOVE_HAZARD_EXT_Y:
            if (c[t] < MAX_MOVEHAZARD) {
                int i = c[t]++; auto& h = stage.moveHazardsExtY[i];
                h.rect = o.rect; h.ismoved = false; h.triggerd = false;
                h.raiseHeight = o.params[0]; h.startY = o.rect.y; h.moveSpeed = o.params[1];
                h.dir = (o.params[2] < 0.0f) ? -1 : 1; // 負値=下向き, 正値=上向き
                stage.moveHazardsExtYInit[i] = h;
            }break;
        case EditorObjectType::MOVE_DOWN_HAZARD_EXT_Y:
            if (c[t] < MAX_MOVEHAZARD) {
                int i = c[t]++; auto& h = stage.moveDownHazardsExtY[i];
                h.rect = o.rect; h.ismoved = false; h.triggerd = false;
                h.raiseHeight = o.params[0]; h.startY = o.rect.y; h.moveSpeed = o.params[1]; h.dir = 1;
                h.tolelance = o.params[2];
                stage.moveDownHazardsExtYInit[i] = h;
            }break;
        case EditorObjectType::MOVE_HAZARD_RIGHT_X:
            if (c[t] < MAX_MOVEHAZARD) {
                int i = c[t]++; auto& h = stage.moveHazardsRight[i];
                h.rect = o.rect; h.ismoved = false; h.triggerd = false;
                h.raiseWidth = o.params[0]; h.startX = o.rect.x; h.moveSpeed = o.params[1];
                h.timer = 0; h.delay = o.params[2];
                stage.moveHazardsRightInit[i] = h;
            }break;
        case EditorObjectType::MOVE_HAZARD_EXT_X:
            if (c[t] < MAX_MOVEHAZARD) {
                int i = c[t]++; auto& h = stage.moveHazardsExtX[i];
                h.rect = o.rect; h.ismoved = false; h.triggerd = false;
                h.raiseWidth = o.params[0]; h.startX = o.rect.x; h.moveSpeed = o.params[1];
                h.timer = 0; h.delay = o.params[2];
                stage.moveHazardsExtXInit[i] = h;
            }break;
        case EditorObjectType::TRACKING_HAZARD:
            if (c[t] < MAX_HAZARDS) {
                int i = c[t]++; auto& h = stage.trackingHazards[i];
                h.rect = o.rect; h.startPos = { o.rect.x,o.rect.y };
                h.speed = o.params[0]; h.trackingRange = o.params[1]; h.maxDistance = o.params[2];
                h.isTracking = false; h.returnToStart = (o.params[3] != 0.0f);
                stage.trackingHazardsInit[i] = h;
            }break;
        case EditorObjectType::ROTATING_BALL:
            if (c[t] < MAX_HAZARDS) {
                int i = c[t]++; auto& b = stage.rotatingBalls[i];
                b.center = { o.rect.x + o.rect.width / 2, o.rect.y + o.rect.height / 2 };
                b.radius = o.params[0]; b.angle = 0; b.angularSpeed = o.params[1];
                b.armLength = (o.params[2] > 0.0f) ? o.params[2] : o.rect.width / 2.0f;
                stage.rotatingBallsInit[i] = b;
            }break;
        case EditorObjectType::MOVE_ROTATING_BALL:
            if (c[t] < MAX_HAZARDS) {
                int i = c[t]++; auto& b = stage.moveRotatingBalls[i];
                b.center = { o.rect.x + o.rect.width / 2, o.rect.y + o.rect.height / 2 };
                b.radius = o.params[0]; b.angle = 0; b.angularSpeed = o.params[1]; b.armLength = (o.params[4] > 0.0f) ? o.params[4] : o.rect.width / 2.0f;
                b.gravity = o.params[2]; b.reverse = (o.params[3] != 0.0f);
                stage.moveRotatingBallsInit[i] = b;
            }break;
        case EditorObjectType::ROLLING_BALL:
            if (c[t] < MAX_HAZARDS) {
                int i = c[t]++;
                auto& b = stage.rollingBalls[i];

                b = RollingBall{};

                float r = (o.params[0] > 0.0f) ? o.params[0] : 15.0f;
                b.center = { o.rect.x + o.rect.width / 2, o.rect.y + o.rect.height / 2 };
                b.radius = r;
                b.rollSpeed = o.params[1];
                b.rollDerection = (fabsf(o.params[2]) < 0.001f) ? -1.0f : o.params[2];

                b.angle = 0.0f;
                b.vel = { 0.0f, 0.0f };
                b.onGround = false;

                b.waitForCursorClick = (o.params[3] != 0.0f); // ★
                b.activated = !b.waitForCursorClick;          // ★ 待機なら非アクティブ

                stage.rollingBallsInit[i] = b;
            } break;
        case EditorObjectType::FALLING_PLATFORM:
            if (c[t] < MAX_FALLING) {
                int i = c[t]++; auto& f = stage.fallingPlatforms[i];
                f.rect = o.rect; f.isfalling = false; f.triggered = false;
                f.fallDelay = o.params[0];
                f.timer = 0;
                f.fallSpeed = o.params[1];
                f.detectMode = (int)o.params[2];
                if (f.detectMode < 0 || f.detectMode > 2) f.detectMode = 0;
                stage.fallingPlatformsInit[i] = f;
            }break;
        

        case EditorObjectType::UPRISING_PLATFORM:
            if (c[t] < MAX_UPRISING) {
                int i = c[t]++; auto& u = stage.upRisingPlatforms[i];
                u.rect = o.rect; u.isrising = false; u.triggered = false;
                u.riseDelay = o.params[0]; u.timer = 0; u.riseSpeed = o.params[1]; u.timeOnUpRising = 0;
                stage.upRisingPlatformsInit[i] = u;
            }break;
        case EditorObjectType::UP_DOWN_PLATFORM:
            if (c[t] < MAX_UPRISING) {
                int i = c[t]++; auto& u = stage.upDouwnPlatforms[i];
                u.rect = o.rect; u.isrising = false;
                u.riseHeight = o.params[0]; u.riseSpeed = o.params[1];
                u.fallSpeed = o.params[2]; u.upperY = u.rect.y - u.riseHeight; u.lowerY = u.rect.y;
                u.timer = 0; u.stopTimer = o.params[3]; u.goingUp = true; u.onUpDownPlatform = false;
                stage.upDouwnPlatformsInit[i] = u;
            }break;
        case EditorObjectType::SWITCH_PLATFORM:
            if (c[t] < MAX_PLATFORMS) {
                int i = c[t]++; auto& s = stage.switchPlatforms[i];
                s.rect = o.rect;
                // 対応するSWITCH_BUTTONがあればそのrectを使う、なければフォールバック
                s.SwitchRect = (switchBtnIdx < (int)switchBtnRects.size())
                    ? switchBtnRects[switchBtnIdx++]
                    : Rectangle{ o.rect.x, o.rect.y - 40, 40, 40 };
                s.speed = { o.params[0], o.params[1] }; s.MaX = o.params[2];
                s.startX = o.rect.x; s.switchedOn = false;
                stage.switchplatformsInit[i] = s;
            }break;
        case EditorObjectType::SWITCH_BUTTON:
            // SWITCH_PLATFORM に割り当てられなかった余剰ボタンは
            // カーソルクリック用ボタン(CURSOR_BOTTOM)として出力する
            if (switchBtnSeen >= switchPlatformTotal) {
                int ct = (int)EditorObjectType::CURSOR_BOTTOM;
                if (c[ct] < MAX_PLATFORMS) {
                    int i = c[ct]++;
                    auto& cb = stage.cursorBottoms[i];
                    cb.rect = o.rect;
                    cb.targetRollingBall = -1;
                    cb.oneShot = true;
                    cb.maxDistance = 0.0f;
                    cb.isActive = false;
                    cb.triggered = false;
                    stage.cursorBottomsInit[i] = cb;
                }
            }
            switchBtnSeen++;
            break;
        case EditorObjectType::FALLING_TEXT:
            if (c[t] < MAX_FALLINGTEXT) {
                int i = c[t]++;
                stage.fallingTexts[i].rect = o.rect;
                stage.fallingTexts[i].fallDelay = o.params[0];
                stage.fallingTexts[i].gravity = o.params[1];
                stage.fallingTexts[i].tolerance = o.params[2];
                stage.fallingTexts[i].fontSize = (int)o.params[3];
                stage.fallingTextsInit[i] = stage.fallingTexts[i];
            }break;
        case EditorObjectType::EXIT_DOOR:
            if (c[t] < Stage::MAX_EXITDOORS) {
                int i = c[t]++;
                stage.exitDoors[i].rect = o.rect; stage.exitDoors[i].targetStage = (int)o.params[0];
            }break;
        case EditorObjectType::WARP_HOLE:
            if (c[t] < Stage::MAX_WARPS) {
                int i = c[t]++;
                stage.warps[i].rect = o.rect;
                stage.warps[i].place = { o.params[0], o.params[1] };
                stage.warps[i].warped = false;
            }break;
        case EditorObjectType::LAYER_DOOR:
            if (c[t] < Stage::MAX_LAYERDOORS) {
                int i = c[t]++; auto& d = stage.layerDoors[i];
                d.frontRect = o.rect; d.backRect = o.rect;
            }break;
        case EditorObjectType::RESPAWN:
            stage.respawnPoint = { o.rect.x, o.rect.y };
            stage.hasRespawnPoint = true;
            break;
        case EditorObjectType::COMMENT_BLOCK:
            if (c[t] < Stage::MAX_COMMENT_BLOCKS) {
                int i = c[t]++; auto& cb = stage.commentBlocks[i];
                cb.rect = o.rect;
                // '@' で始まる場合は DialogManager のキーとして解決
                if (!o.text.empty() && o.text[0] == '@') {
                    const std::string key = o.text.substr(1);
                    auto& dm = DialogManager::Instance();
                    cb.message = dm.Has(key) ? dm.Get(key, "") : o.text;
                } else {
                    cb.message = o.text;
                }
                cb.duration = o.params[0];
                cb.triggered = false;
                cb.cooldown = 0.0f;
                stage.commentBlocksInit[i] = cb;
            }break;

        case EditorObjectType::DEATH_BLOCK:
            if (c[t] < MAX_HAZARDS) {
                int i = c[t]++;
                stage.deathBlocks[i] = o.rect;
                stage.deathBlocksInit[i] = o.rect;
            }break;

        case EditorObjectType::OJISAN_PUNCH_AREA:
            if (c[t] < Stage::MAX_OJISAN_PUNCH_AREAS) {
                int i = c[t]++;
                stage.ojisanPunchAreas[i] = o.rect;
                stage.ojisanPunchTriggered[i] = false;
            } break;

        case EditorObjectType::CURSOR_BOTTOM:
            if (c[t] < MAX_PLATFORMS) {
                int i = c[t]++;
                auto& cb = stage.cursorBottoms[i];
                cb.rect = o.rect;
                cb.targetRollingBall = (int)o.params[0];
                cb.oneShot = (o.params[1] != 0.0f);
                cb.maxDistance = o.params[2];
                cb.isActive = false;
                cb.triggered = false;
                stage.cursorBottomsInit[i] = cb;
            }break;
        case EditorObjectType::SPIKE_BOUNCER:
            if (c[t] < Stage::MAX_SPIKE_BOUNCERS) {
                int i = c[t]++; auto& sb = stage.spikeBouncers[i];
                sb.rect = o.rect;
                sb.bounceVelosity = { o.params[0], o.params[1] };
                sb.startY = o.rect.y;
                stage.spikeBouncer​Init[i] = sb;
            }break;

        case EditorObjectType::SPRING:
            if (c[t] < Stage::MAX_SPRINGS) {
                int i = c[t]++; auto& sp = stage.springs[i];
                sp.rect = o.rect;
                sp.bounceVelocity = { o.params[0], o.params[1] };
                sp.isActive = true;
                sp.pickedUpByPlayer = false;
                stage.springsInit[i] = sp;
            }break;

        case EditorObjectType::CRANE_LAUNCH_PAD:
            if (c[t] < Stage::MAX_CRANES) {
                int i = c[t]++; auto& cl = stage.craneLaunchPads[i];
                cl.rect = o.rect;
                cl.launchVelY = o.params[0];
                cl.triggered = false;
                stage.craneLaunchPadsInit[i] = cl;
            }break;

        case EditorObjectType::CRANE:
            if (c[t] < Stage::MAX_CRANES) {
                int i = c[t]++; auto& cr = stage.cranes[i];
                cr.bodyRect = o.rect;
                cr.ceilingY = o.rect.y;
                cr.maxArmLength = o.params[0];
                cr.detectRangeX = o.params[1];
                cr.carrySpeedX = o.params[2];
                cr.carryDir = (int)o.params[3];
                cr.descendSpeed = 300.0f;
                cr.carryDist = (o.params[4] > 0.0f) ? o.params[4] : 2000.0f;
                cr.armLength = 0.0f;
                cr.carriedX = 0.0f;
                cr.state = CraneState::IDLE;
                stage.cranesInit[i] = cr;
            }break;
            
        case EditorObjectType::TEMP_FLOOR:
            if (c[t] < Stage::MAX_TEMP_FLOORS) {
                int i = c[t]++;
                auto& tf = stage.tempFloors[i];
                tf.rect = o.rect;
                tf.showSec = (o.params[0] > 0.0f) ? o.params[0] : 2.0f;
                tf.timer = 0.0f;
                tf.visible = false;
                stage.tempFloorsInit[i] = tf;
            } break;

        case EditorObjectType::TEMP_FLOOR_SWITCH:
            if (c[t] < Stage::MAX_TEMP_FLOORS) {
                int i = c[t]++;
                auto& sw = stage.tempFloorSwitches[i];
                sw.rect = o.rect;
                sw.targetFloor = (int)o.params[0];
                sw.oneShot = (o.params[1] != 0.0f);
                sw.triggered = false;
                sw.hover = false;
                stage.tempFloorSwitchesInit[i] = sw;
            } break;

        default: break;
        }
    }
    stage.platformCount = c[0];  stage.backPlatformCount = c[1];
    stage.hazardCount = c[2];    stage.touchBreakBlockCount = c[3];
    stage.bottomBreakBlockCount = c[4]; stage.breakableBlockCount = c[5];
    stage.elevatorCount = c[6];  stage.gravityBlockCount = c[7];
    stage.buttonBlockCount = c[8]; stage.itemBlockCount = c[9];
    stage.icePlatformCount = c[10]; stage.moveDownPlatformCount = c[11]
    ;
    stage.moveUpPlatformCount = c[12]; stage.jumpPlatfromCount = c[13];
    stage.batteryHumanCount = c[14]; stage.cursorPlatformCount = c[15];
    stage.magnetCount = c[16];   stage.knockBackWallCount = c[17];
    stage.splitPlatformCount = c[18]; stage.circlePlatformCount = c[19];
    stage.movePlatformCountX = c[20]; stage.moveCount = c[21];
    stage.moveExtYCount = c[22]; stage.moveDownHazardExtYCount = c[23];
    stage.moveHazardRightCount = c[24]; stage.moveExtXCount = c[25];
    stage.trackingHazardCount = c[26]; stage.rotatingBallCount = c[27];
    stage.moveRotatingBallCount = c[28]; stage.rollingBallCount = c[29];
    stage.fallingCount = c[30];  stage.upRisingCount = c[31];
    stage.upDownCount = c[32];   stage.clearsCount = c[33];
    stage.clearsXCount = c[34];  stage.switchPlatformCount = c[35];
	stage.fallingTextCount = c[36]; stage.exitDoorCount = c[37];
	stage.layerDoorCount = c[38];
	stage.commentBlockCount = c[41];
	stage.cursorBottomCount = c[42];
	stage.deathBlockCount = c[43];
	stage.spikeBouncer​Count = c[44];
	stage.springCount = c[45];
	stage.craneLaunchPadCount = c[46];
	stage.craneCount = c[47];
	stage.ojisanPunchAreaCount = c[48];
	stage.warpCount = c[49];
	stage.tempFloorCount = c[(int)EditorObjectType::TEMP_FLOOR];              // 51
	stage.tempFloorSwitchCount = c[(int)EditorObjectType::TEMP_FLOOR_SWITCH]; // 52

	StageThemeLoadAll(stage.theme,
        "assets/images/stage/stage_1/ground1.png",
        "assets/images/stage/stage_1/ground2.png",
        "assets/images/stage/stage_1/ground3.png",
        "assets/images/stage/stage_1/ground4.png",
        50.0f);

    enemyManager.Init();

	for (const auto& pe : ed.placedEnemies) {// placedEnemies を Stage に反映
        enemyManager.Spawn(pe.type, pe.pos);
		auto& e = enemyManager.enemies.back();// 新しい敵を追加配列の後ろに追加

		const auto& info = EdGetEnemyTypeInfo(pe.type);// 敵のタイプ情報を取得

        if (info.count > 0)e.speed = pe.params[0];

        // パトロール範囲を配置位置から計算
        float patrolDist = (info.count > 1) ? pe.params[1] : 200.0f;
        e.patrolMinX = pe.pos.x - patrolDist;
        e.patrolMaxX = pe.pos.x + patrolDist;
        e.patrolMinY = pe.pos.y - patrolDist;
        e.patrolMaxY = pe.pos.y + patrolDist;

        // params[2] = hp（WALKERはindex2）
        if (info.count > 2) e.hp = pe.params[2];
    
    }
    enemyManager.saveEnemiesInit();
}

// ================================================================
//  Import (Stage -> editor)
// ================================================================
void EditorImportFromStage(StageEditor& ed, const Stage& s) {
    ed.objects.clear();
    ed.undoStack.clear();
    ed.propSelectedIdx = -1;
    ed.propEditingParam = -1;

#define IMPORT_RAW_P(TYPE, arr, cnt) \
        for(int i=0;i<cnt;i++){PlacedObject o={EditorObjectType::TYPE,arr[i]}; \
        InitDefaultParams(o);ed.objects.push_back(o);}
#define IMPORT_RECT_P(TYPE, arr, cnt) \
        for(int i=0;i<cnt;i++){PlacedObject o={EditorObjectType::TYPE,arr[i].rect}; \
        InitDefaultParams(o);ed.objects.push_back(o);}

    IMPORT_RAW_P(PLATFORM, s.platforms, s.platformCount)
        IMPORT_RAW_P(BACK_PLATFORM, s.backPlatforms, s.backPlatformCount)
        IMPORT_RAW_P(HAZARD, s.hazards, s.hazardCount)
        IMPORT_RECT_P(TOUCH_BREAK_BLOCK, s.touchBreakBlocks, s.touchBreakBlockCount)
        IMPORT_RECT_P(BOTTOM_BREAK_BLOCK, s.bottomBreakBlocks, s.bottomBreakBlockCount)
        IMPORT_RECT_P(BREAKABLE_BLOCK, s.breakableBlocks, s.breakableBlockCount)
        IMPORT_RECT_P(ELEVATOR, s.elevators, s.elevatorCount)
        IMPORT_RECT_P(GRAVITY_BLOCK, s.gravityBlocks, s.gravityBlockCount)
        IMPORT_RECT_P(ITEM_BLOCK, s.itemBlocks, s.itemBlockCount)
        IMPORT_RECT_P(MOVE_DOWN_PLATFORM, s.moveDownPlatforms, s.moveDownPlatformCount)
        IMPORT_RECT_P(MOVE_UP_PLATFORM, s.moveUpPlatforms, s.moveUpPlatformCount)
        IMPORT_RECT_P(CLEAR_BLOCK, s.clearBlocks, s.clearsCount)
        IMPORT_RECT_P(CLEAR_BLOCK_X, s.clearBlocksX, s.clearsXCount)
        IMPORT_RAW_P(DEATH_BLOCK, s.deathBlocks, s.deathBlockCount)
        IMPORT_RAW_P(OJISAN_PUNCH_AREA, s.ojisanPunchAreas, s.ojisanPunchAreaCount)

        for (int i = 0; i < s.spikeBouncer​Count; i++) {
            PlacedObject o = { EditorObjectType::SPIKE_BOUNCER, s.spikeBouncers[i].rect };
            o.params[0] = s.spikeBouncers[i].bounceVelosity.x;
            o.params[1] = s.spikeBouncers[i].bounceVelosity.y;
            ed.objects.push_back(o);
        }
    for (int i = 0; i < s.icePlatformCount; i++) {
        PlacedObject o = { EditorObjectType::ICE_PLATFORM,s.icePlatforms[i].rect };
        o.params[0] = s.icePlatforms[i].friction; o.params[1] = s.icePlatforms[i].iceAccel;
        o.params[2] = s.icePlatforms[i].maxSlideSpeed; ed.objects.push_back(o);
    }
    for (int i = 0; i < s.jumpPlatfromCount; i++) {
        PlacedObject o = { EditorObjectType::JUMP_PLATFORM,s.jumpPlatfroms[i].rect };
        o.params[0] = s.jumpPlatfroms[i].jumpVelocity.x;
        o.params[1] = s.jumpPlatfroms[i].jumpVelocity.y;
        o.params[2] = s.jumpPlatfroms[i].stopTimer;
        o.params[3] = s.jumpPlatfroms[i].delay;
        o.params[4] = s.jumpPlatfroms[i].withdraw;
        o.params[5] = s.jumpPlatfroms[i].jumpSpeed;
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.batteryHumanCount; i++) {
        PlacedObject o = { EditorObjectType::BATTERY_HUMAN,s.batteryHumans[i].rect };
        o.params[0] = s.batteryHumans[i].BatteryVelocity.x;
        o.params[1] = s.batteryHumans[i].BatteryVelocity.y;
        o.params[2] = s.batteryHumans[i].delay;
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.cursorPlatformCount; i++) {
        PlacedObject o = { EditorObjectType::CURSOR_PLATFORM,s.cursorPlatforms[i].rect };
        o.params[0] = s.cursorPlatforms[i].followSpeed; o.params[1] = s.cursorPlatforms[i].maxDistance;
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.magnetCount; i++) {
        PlacedObject o = { EditorObjectType::MAGNET,s.magnets[i].rect };
        o.params[0] = s.magnets[i].Force; o.params[1] = s.magnets[i].maxDistance;
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.knockBackWallCount; i++) {
        PlacedObject o = { EditorObjectType::KNOCKBACK_WALL,s.knockBackWalls[i].rect };
        o.params[0] = s.knockBackWalls[i].knockBackVelocity.x;
        o.params[1] = s.knockBackWalls[i].knockBackVelocity.y;
        o.params[2] = s.knockBackWalls[i].withdraw;
        o.params[3] = s.knockBackWalls[i].delay;
        o.params[4] = s.knockBackWalls[i].limitStop;
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.cursorBottomCount; i++) {
        PlacedObject o = { EditorObjectType::CURSOR_BOTTOM, s.cursorBottoms[i].rect };
        o.params[0] = (float)s.cursorBottoms[i].targetRollingBall;
        o.params[1] = s.cursorBottoms[i].oneShot ? 1.0f : 0.0f;
        o.params[2] = s.cursorBottoms[i].maxDistance;
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.spikeBouncer​Count; i++) {
        PlacedObject o = { EditorObjectType::SPIKE_BOUNCER, s.spikeBouncers[i].rect };
        o.params[0] = s.spikeBouncers[i].bounceVelosity.x;
        o.params[1] = s.spikeBouncers[i].bounceVelosity.y;
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.splitPlatformCount; i++) {
        PlacedObject o = { EditorObjectType::SPLIT_PLATFORM,s.splitPlatforms[i].base };
        o.params[0] = s.splitPlatforms[i].maxGap; o.params[1] = s.splitPlatforms[i].splitSpeed;
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.circlePlatformCount; i++) {
        auto& cp = s.circlePlatforms[i]; float al = cp.armLength;
        PlacedObject o = { EditorObjectType::CIRCLE_PLATFORM,{cp.center.x - al,cp.center.y - al,al * 2,al * 2} };
        o.params[0] = cp.radius; o.params[1] = cp.angleSpeed; ed.objects.push_back(o);
    }
    for (int i = 0; i < s.movePlatformCountX; i++) {
        PlacedObject o = { EditorObjectType::MOVE_PLATFORM_X,s.movePlatformsX[i].rect };
        o.params[0] = s.movePlatformsX[i].moveSpeed; o.params[1] = s.movePlatformsX[i].moveDistance;
        o.params[2] = s.movePlatformsX[i].delay; ed.objects.push_back(o);
    }
    for (int i = 0; i < s.moveCount; i++) {
        PlacedObject o = { EditorObjectType::MOVE_HAZARD,s.moveHazards[i].rect };
        o.params[0] = s.moveHazards[i].raiseHeight; o.params[1] = s.moveHazards[i].moveSpeed;
        ed.objects.push_back(o);
    }
    // ★ MOVE_HAZARD_EXT_Y: dir を params[2] に保存
    for (int i = 0; i < s.moveExtYCount; i++) {
        PlacedObject o = { EditorObjectType::MOVE_HAZARD_EXT_Y, s.moveHazardsExtY[i].rect };
        o.params[0] = s.moveHazardsExtY[i].raiseHeight;
        o.params[1] = s.moveHazardsExtY[i].moveSpeed;
        o.params[2] = (float)s.moveHazardsExtY[i].dir; // 1=上向き, -1=下向き
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.moveDownHazardExtYCount; i++) {
        PlacedObject o = { EditorObjectType::MOVE_DOWN_HAZARD_EXT_Y,s.moveDownHazardsExtY[i].rect };
        o.params[0] = s.moveDownHazardsExtY[i].raiseHeight;
        o.params[1] = s.moveDownHazardsExtY[i].moveSpeed;
        o.params[2] = s.moveDownHazardsExtY[i].tolelance;
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.moveHazardRightCount; i++) {
        PlacedObject o = { EditorObjectType::MOVE_HAZARD_RIGHT_X,s.moveHazardsRight[i].rect };
        o.params[0] = s.moveHazardsRight[i].raiseWidth; o.params[1] = s.moveHazardsRight[i].moveSpeed;
        o.params[2] = s.moveHazardsRight[i].delay; ed.objects.push_back(o);
    }
    for (int i = 0; i < s.moveExtXCount; i++) {
        PlacedObject o = { EditorObjectType::MOVE_HAZARD_EXT_X,s.moveHazardsExtX[i].rect };
        o.params[0] = s.moveHazardsExtX[i].raiseWidth; o.params[1] = s.moveHazardsExtX[i].moveSpeed;
        o.params[2] = s.moveHazardsExtX[i].delay; ed.objects.push_back(o);
    }
    for (int i = 0; i < s.trackingHazardCount; i++) {
        PlacedObject o = { EditorObjectType::TRACKING_HAZARD,s.trackingHazards[i].rect };
        o.params[0] = s.trackingHazards[i].speed;
        o.params[1] = s.trackingHazards[i].trackingRange;
        o.params[2] = s.trackingHazards[i].maxDistance;
        o.params[3] = s.trackingHazards[i].returnToStart ? 1.0f : 0.0f;
        ed.objects.push_back(o);
    }
    // ★ ROTATING_BALL: armLen を params[2] から適用
    for (int i = 0; i < s.rotatingBallCount; i++) {
        auto& b = s.rotatingBalls[i];
        PlacedObject o = { EditorObjectType::ROTATING_BALL,
            { b.center.x - b.armLength, b.center.y - b.armLength, b.armLength * 2, b.armLength * 2 } };
        o.params[0] = b.radius;
        o.params[1] = b.angularSpeed;
        o.params[2] = b.armLength; // ★
        ed.objects.push_back(o);
    }
    // ★ MOVE_ROTATING_BALL: armLength を params[4] に保存
    for (int i = 0; i < s.moveRotatingBallCount; i++) {
        auto& b = s.moveRotatingBalls[i];
        PlacedObject o = { EditorObjectType::MOVE_ROTATING_BALL,
            { b.center.x - b.armLength, b.center.y - b.armLength, b.armLength * 2, b.armLength * 2 } };
        o.params[0] = b.radius;
        o.params[1] = b.angularSpeed;
        o.params[2] = b.gravity;
        o.params[3] = b.reverse ? 1.0f : 0.0f;
        o.params[4] = b.armLength; // ★
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.rollingBallCount; i++) {
        float r = s.rollingBalls[i].radius > 0 ? s.rollingBalls[i].radius : 15.0f;
        PlacedObject o = { EditorObjectType::ROLLING_BALL,
            { s.rollingBalls[i].center.x - r, s.rollingBalls[i].center.y - r, r * 2, r * 2 } };
        o.params[0] = r;
        o.params[1] = s.rollingBalls[i].rollSpeed;
        o.params[2] = s.rollingBalls[i].rollDerection;
        o.params[3] = s.rollingBalls[i].waitForCursorClick ? 1.0f : 0.0f; // ★
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.fallingCount; i++) {
        PlacedObject o = { EditorObjectType::FALLING_PLATFORM, s.fallingPlatforms[i].rect };
        o.params[0] = s.fallingPlatforms[i].fallDelay;
        o.params[1] = s.fallingPlatforms[i].fallSpeed;
        o.params[2] = (float)s.fallingPlatforms[i].detectMode;
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.upRisingCount; i++) {
        PlacedObject o = { EditorObjectType::UPRISING_PLATFORM,s.upRisingPlatforms[i].rect };
        o.params[0] = s.upRisingPlatforms[i].riseDelay; o.params[1] = s.upRisingPlatforms[i].riseSpeed;
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.upDownCount; i++) {
        PlacedObject o = { EditorObjectType::UP_DOWN_PLATFORM,s.upDouwnPlatforms[i].rect };
        o.params[0] = s.upDouwnPlatforms[i].riseHeight; o.params[1] = s.upDouwnPlatforms[i].riseSpeed;
        o.params[2] = s.upDouwnPlatforms[i].fallSpeed;  o.params[3] = s.upDouwnPlatforms[i].stopTimer;
        ed.objects.push_back(o);
    }
    // SWITCH_PLATFORM: 移動床 + 対応するボタンをセットでインポート
    for (int i = 0; i < s.switchPlatformCount; i++) {
        PlacedObject o = { EditorObjectType::SWITCH_PLATFORM, s.switchPlatforms[i].rect };
        o.params[0] = s.switchPlatforms[i].speed.x;
        o.params[1] = s.switchPlatforms[i].speed.y;
        o.params[2] = s.switchPlatforms[i].MaX;
        ed.objects.push_back(o);

        // 対応するボタンも復元
        PlacedObject btn = { EditorObjectType::SWITCH_BUTTON, s.switchPlatforms[i].SwitchRect };
        InitDefaultParams(btn);
        ed.objects.push_back(btn);
    }
    for (int i = 0; i < s.fallingTextCount; i++) {
        PlacedObject o = { EditorObjectType::FALLING_TEXT, s.fallingTexts[i].rect };
        o.params[0] = s.fallingTexts[i].fallDelay;
        o.params[1] = s.fallingTexts[i].gravity;
        o.params[2] = s.fallingTexts[i].tolerance;
        o.params[3] = (float)s.fallingTexts[i].fontSize;
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.exitDoorCount; i++) {
        PlacedObject o = { EditorObjectType::EXIT_DOOR,s.exitDoors[i].rect };
        o.params[0] = (float)s.exitDoors[i].targetStage; ed.objects.push_back(o);
    }
    for (int i = 0; i < s.warpCount; i++) {
        PlacedObject o = { EditorObjectType::WARP_HOLE, s.warps[i].rect };
        o.params[0] = s.warps[i].place.x;
        o.params[1] = s.warps[i].place.y;
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.layerDoorCount; i++) {
        PlacedObject o = { EditorObjectType::LAYER_DOOR,s.layerDoors[i].frontRect };
        InitDefaultParams(o); ed.objects.push_back(o);
    }
    // ELEVATOR (手動インポート: speed/rangeUp/rangeDown を復元)
    for(int i=0;i<s.elevatorCount;i++){
        PlacedObject o={EditorObjectType::ELEVATOR, s.elevators[i].rect};
        o.params[0] = s.elevators[i].speed;
        o.params[1] = s.elevators[i].rect.y - s.elevators[i].upperY;  // rangeUp
        o.params[2] = s.elevators[i].lowerY - s.elevators[i].rect.y;  // rangeDown
        ed.objects.push_back(o);
    }

    for (int i = 0; i < s.springCount; i++) {  
        PlacedObject o = { EditorObjectType::SPRING, s.springs[i].rect };
        o.params[0] = s.springs[i].bounceVelocity.x;
        o.params[1] = s.springs[i].bounceVelocity.y;
        ed.objects.push_back(o);
    }

    // リスポーン地点
    if (s.hasRespawnPoint) {
        PlacedObject o = { EditorObjectType::RESPAWN,
            { s.respawnPoint.x, s.respawnPoint.y, 60.0f, 70.0f } };
        InitDefaultParams(o);
        ed.objects.push_back(o);
    }
	// TEMP_FLOOR と TEMP_FLOアーSWITCH を復元
    for (int i = 0; i < s.tempFloorCount; i++) {
        PlacedObject o = { EditorObjectType::TEMP_FLOOR, s.tempFloors[i].rect };
        o.params[0] = s.tempFloors[i].showSec;
        ed.objects.push_back(o);
    }
    for (int i = 0; i < s.tempFloorSwitchCount; i++) {
        PlacedObject o = { EditorObjectType::TEMP_FLOOR_SWITCH, s.tempFloorSwitches[i].rect };
        o.params[0] = (float)s.tempFloorSwitches[i].targetFloor;
        o.params[1] = s.tempFloorSwitches[i].oneShot ? 1.0f : 0.0f;
        ed.objects.push_back(o);
    }
#undef IMPORT_RAW_P
#undef IMPORT_RECT_P
}

// ================================================================
bool EditorSaveJSON(const StageEditor& ed, const char* filename) {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) return false;
    ofs << "{\n  \"objects\": [\n";
    for (size_t i = 0; i < ed.objects.size(); i++) {
        const auto& o = ed.objects[i];
        ofs << "    { \"type\": \"" << GetNameEN((int)o.type)
            << "\", \"typeId\": " << (int)o.type
            << ", \"x\": " << o.rect.x << ", \"y\": " << o.rect.y
            << ", \"w\": " << o.rect.width << ", \"h\": " << o.rect.height << " }";
        if (i + 1 < ed.objects.size()) ofs << ",";
        ofs << "\n";
    }
    ofs << "  ]\n}\n";
    return true;
}
bool EditorSaveCSV(const StageEditor& ed, const char* filename) {
    // 指定されたファイル名で書き込み用ファイルを開く
    std::ofstream ofs(filename);

    // ファイルを開けなかった場合は保存失敗
    if (!ofs.is_open()) return false;

    // 通常オブジェクト用の見出し行を書き込む
    ofs << "type,typeId,x,y,w,h,p0,p1,p2,p3,p4,p5,text\n";

    // 配置済みの通常オブジェクトを1つずつ保存する
    for (const auto& o : ed.objects) {
        // オブジェクト名、種類番号、位置、大きさを書き込む
        ofs << GetNameEN((int)o.type) << "," << (int)o.type << ","
            << o.rect.x << "," << o.rect.y << ","
            << o.rect.width << "," << o.rect.height;

        // オブジェクトのパラメータをすべて保存する
        for (int i = 0; i < MAX_OBJ_PARAMS; i++) {
            ofs << "," << o.params[i];
        }

        // オブジェクトのテキストを保存する
        // ダブルクォーテーションで囲んでCSVに書き込む
        ofs << ",\"" << o.text << "\"\n";
    }

    // ここから敵データであることを示す目印
    ofs << "[ENEMIES]\n";

    // 敵データ用の見出し行
    ofs << "enemyTypeId,posX,posY,p0,p1,p2,p3,p4,p5\n";

    // 配置済みの敵を1体ずつ保存する
    for (const auto& e : ed.placedEnemies) {
        // 敵の種類番号と配置位置を保存する
        ofs << (int)e.type << "," << e.pos.x << "," << e.pos.y;

        // 敵のパラメータをすべて保存する
        for (int i = 0; i < MAX_OBJ_PARAMS; i++) {
            ofs << "," << e.params[i];
        }

        // 敵1体分を書き終えたので改行する
        ofs << "\n";
    }

    // ここまで来たら保存成功
    return true;
}

bool EditorLoadCSV(StageEditor& ed, const char* filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) return false;
    ed.objects.clear();
	ed.placedEnemies.clear();// 敵データもクリア
    ed.propSelectedIdx = -1;
    ed.propEditingParam = -1;
    ed.propEditingText = false;
    ed.selectedEnemyIdx = -1;
    std::string line;
    std::getline(ifs, line); // ヘッダースキップ

	bool inEnemySection = false; // 敵データセクションに入ったかどうかのフラグ
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        //[ENEMIES]セクションに到達したら敵データの読み込みに切り替える
        if (line == "[ENEMIES]") {
            inEnemySection = true;
			std::getline(ifs, line);// 敵データのヘッダースキップ
            continue;
        }
        if (inEnemySection) {
            std::istringstream ss(line);
			int typeId; float posX, posY;// 敵の種類番号と配置位置を読み取る
			char cm;// カンマ区切りの読み取り用
            ss >> typeId >> cm >> posX >> cm >> posY;

            PlacedEnemy e;
            e.type = (EnemyType)typeId;
            e.pos = { posX,posY };
            InitDefaultEnemyParams(e);
            for (int i = 0; i < MAX_OBJ_PARAMS; i++) {
                float pv;
                if (ss >> cm >> pv) 
                    /*カンマと数値をちゃんと読み取れたら true
                      読み取れなかったら false*/
                    e.params[i] = pv;
            }
            ed.placedEnemies.push_back(e);
            continue;
        }


        std::istringstream ss(line);
        std::string tn;
        int tid; float x, y, w, h; char cm;
        std::getline(ss, tn, ',');
        ss >> tid >> cm >> x >> cm >> y >> cm >> w >> cm >> h;
        PlacedObject obj;

        int resolvedTypeId = -1;
        for (int i = 0; i < (int)EditorObjectType::COUNT; i++) {
            if (tn == GetNameEN(i)) {
                resolvedTypeId = i;
                break;
            }
        }

        if (resolvedTypeId >= 0) {
            obj.type = (EditorObjectType)resolvedTypeId;
        }
        else if (tid >= 0 && tid < (int)EditorObjectType::COUNT) {
            obj.type = (EditorObjectType)tid;
        }
        else {
            obj.type = EditorObjectType::PLATFORM;
        }

        obj.rect = { x, y, w, h };
        InitDefaultParams(obj);
        for (int i = 0; i < MAX_OBJ_PARAMS; i++) {
            float pv;
            if (ss >> cm >> pv) obj.params[i] = pv;
        }
        // テキスト列の読み取り（ダブルクォートで囲まれている）
        if (ss.peek() == ',' || ss.peek() == ' ') ss.get();
        std::string textField;
        std::getline(ss, textField);
        // ダブルクォートを除去
        if (textField.size() >= 2 && textField.front() == '"' && textField.back() == '"')
            textField = textField.substr(1, textField.size() - 2);
        obj.text = textField;

        ed.objects.push_back(obj);
    }
    return true;
}