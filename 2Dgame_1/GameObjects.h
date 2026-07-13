#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include"Item.h"
#include"SpriteDatabase.h"

// ================================================================
// GameObjects.h の役割
// ---------------------------------------------------------------
// ・ステージ内で使うギミック構造体を集中定義する。
// ・更新/描画/衝突モジュールはこの共通データを参照する。
// ================================================================
//壊せるブロック
struct BreakableBlock {
    Rectangle rect;
	bool isBroken = false;//壊れているか
};

//アイテムブロック
struct ItemBlock {
    Rectangle rect;
    bool isActive = false;//アイテムが出現したか
    ItemType spawnType = ItemType::jumpPotion;
	bool hasSpawned = false;//アイテムが出現済みか
};

//透明ブロック
struct ClearBlock {
    Rectangle rect;
    bool clearflag = false;
};
//透明ブロック（横）
struct ClearBlockX{
    Rectangle rect;
    bool clearflag = false;

};
//スイッチによって動作する床
struct SwitchPlatform {
    Rectangle rect;
	Rectangle SwitchRect;//スイッチの当たり判定
	Vector2 speed;//移動速度
	float MaX;//最大移動距離200
	float startX;//初期位置
    bool switchedOn;
    



};
//わかれる床のデータ
struct SplitPlatform {
	Rectangle baseInit;//初期化用
	Rectangle base;//分かれる前の床    
	Rectangle left;//分かれたあとの左と右
	Rectangle right;//分かれたあとの左と右

	bool triggered;//踏まれたか
	float offsetX;//どれだけ離れるか
	float maxGap;//最大離れる距離
	float splitSpeed;//分離速度

};

//カーソル追従床
struct CursorPlatform {
    Rectangle rect;
	float followSpeed;//追従速度
    float maxDistance;
    bool isActive;
    bool isDragging = false;
    
    Vector2 GetCenter() const {
        return {
            rect.x + rect.width / 2,
            rect.y + rect.height / 2
        };
	}
};
//カーソルによって作動
struct CursorBottom {
    Rectangle rect;
	float maxDistance;//カーソルからどれだけ離れたら作動するか
    bool isActive;

    int targetRollingBall = -1;
    bool oneShot = true;
	bool triggered = false;//一度作動したら作動しない

    Vector2 GetCenter() const {
        return {
            rect.x + rect.width / 2,
            rect.y + rect.height / 2
        };
	}
};


//磁石（引き寄せ）
struct Magnet {
    Rectangle rect;
    float Force;
    float maxDistance;
    bool isActive;
    bool isDragging = false;

    Vector2 GetCenter() const {
        return {
            rect.x + rect.width / 2,
            rect.y + rect.height / 2
        };
    }
};


//氷床
struct IcePlatform {
    Rectangle rect;
    float friction;        // 摩擦係数
    float iceAccel;        // 現在の滑り速度
    float maxSlideSpeed;   // 最大滑り速度
};

//移動低下床
struct MoveDownPlatform {
    Rectangle rect;
};

//移動上昇床
struct MoveUpPlatform {
    Rectangle rect;
};

//触れると吹っ飛ばす壁
struct KnockBackWall {
    Rectangle rect;
    float startX;
    Vector2 knockBackVelocity;//吹っ飛ばす速度
    bool onTouch=false;//触れたか
	bool twiceTouch = false;//二回目に触れたか
	bool iswithdraw = false;//引っ込んでいるか
    bool isRetrun = false;
    float timer =0.0f; 
    float delay = 0.1f;
    float withdraw=5.0f;//どれくらい壁が引っ込むか
    float stoptimer = 0.0f;
    float limitStop = 0.1f;

    Vector2 getRectposition() const {
        return {
            rect.x, rect.y
        };
    }
};


//ジャンプ台
struct JumpPlatfrom {
    Rectangle rect;
    Vector2 jumpVelocity;
    float startY;
    float stopTimer = 0.4f;
    float timer = 0.0f;
    float delay = 0.1f;
    float withdraw = 3.0f;
    float limitStop = 0.5f;
	float jumpSpeed = 200.0f;
    bool twiceTouch = false;
    bool onTouch = false;
    bool isReturn = false;
    bool isWithdraw = false;

};
//プレイヤーが持ち運べるばね
struct Spring {
    Rectangle rect;
    Vector2 bounceVelocity;
    bool isActive;
    bool pickedUpByPlayer;
};

//人が入ると発射する砲台
struct BatteryHuman {
    Rectangle rect;
	Vector2 BatteryVelocity;
    bool onEnter = false;
    bool isReturn = false;
    bool onLeft = false;
	bool onRight = false;
    float timer = 0.0f;
    float delay = 0.5f;
};

//乗ると動く床のデータ(x方向)
struct MovePlatformX {
    float moveSpeed;
    float moveDistance;
    float timer;
    float delay;
	bool triggerd;//一回踏まれたか
	bool ismoved;//動いているか
	bool onplayer;//プレイヤーが乗っているか
    Rectangle rect;
};

//動くとげ
struct MoveHazard {
    Rectangle rect;
    bool ismoved;
    bool triggerd;
    float raiseHeight;//どれだけあがるか
    float startY;     //初期Y（引っ込んでる値）
    float moveSpeed;
};
//動くとげY拡張
struct MoveHazardExtentionY {
    Rectangle rect;
    bool ismoved;
    bool triggerd;
    float raiseHeight;//どれだけあがるか
    float startY;     //初期Y（引っ込んでる値）
	float moveSpeed;
    int dir = 1;//移動方向
};

//おちてくるとげ
struct MoveDownHazardExtentionY {
    Rectangle rect;
    bool ismoved;
    bool triggerd;
    float raiseHeight;//どれだけあがるか
    float startY;     //初期Y（引っ込んでる値）
    float moveSpeed;
    int dir = 1;//移動方向
	float tolelance = 5.0f;//許容値
};
//動くとげX(ｙ厳密)
struct MoveHazardRightX {
    Rectangle rect;
    bool ismoved;
    bool triggerd;
    float raiseWidth;//どれだけ出るか
    float startX;     //初期Y（引っ込んでる値）
    float moveSpeed;
    float timer;
    float delay;
    int dir = 1;//出現方向（1=右, -1=左）
    float toleranceX = 24.0f;//感知するX方向の許容範囲（プレイヤーが近づいたと判定する距離）
	float toleranceY = 24.0f;//感知するY方向の許容範囲（プレイヤーが近づいたと判定する距離）
};

//追尾するとげ
struct TrackingHazard {
    Rectangle rect;
    Vector2 startPos;
    float speed;
	float trackingRange;
	float maxDistance;//最大追尾距離
	bool isTracking;//追尾中か
	bool returnToStart;//元の位置に戻るか

};

//スパイク型トランポリン
struct SpikeBouncer {
    Rectangle rect;
    Vector2 bounceVelosity;
    float startY;
    float stopTimer = 0.4f;
    float timer = 0.0f;
    float delay = 0.1f;
	float compress = 5.0f;//どれだけ圧縮するか
	float limitStop = 0.5f;//圧縮している時間
    bool twiceTouch = false;
    bool onTouch = false;
    bool isCompressing = false;
    
};
// 落下床のデータ
struct FallingPlatform {
    Rectangle rect;
    bool isfalling;
    bool triggered;   // 一回踏まれたか
    float fallDelay;  // 踏んでから落ちるまで
    float timer;      // 経過時間
    float fallSpeed;  // 落下速度
    int detectMode = 0; // 0 = 両方、１＝プレイヤーのみ、２＝ボールのみ
};

//往復上昇床
struct UpDownPlatform {
    Rectangle rect;
    bool isrising;
    bool riseHeight; // 上昇高さ
    bool riseSpeed;  // 上昇速度
    bool fallSpeed;  // 下降速度
    float upperY;     // 上昇限界Y座標
    float lowerY;     // 下降限界Y座標
	float timer;       // 経過時間
	float stopTimer;      // 停止時間
    bool goingUp; // 上昇中か
	bool onUpDownPlatform; // 上昇下降中か
};

//円軌道床
struct CirclePlatform {
    Vector2 vel = { 0.0f,0.0f };//速度
    Vector2 center; // 回転中心
    float radius;
    float angle;
    float angleSpeed;
    float armLength;
	bool onPlayer = false;
	Vector2 prevPos = { 0.0f,0.0f };//前のフレームの位置
    Vector2 getRectposition() const {
        return{
            center.x + armLength * cosf(angle),
            center.y + armLength * sinf(angle),
        };
    }

    Rectangle GetRect() const {
        Vector2 pos = getRectposition();
        return Rectangle{
            pos.x - radius, pos.y - radius, radius * 2, radius * 2
        };
    }
};

// 上昇床のデータ
struct UpRisingPlatform {
    Rectangle rect;
    bool isrising;
    bool triggered;   // 一回踏まれたか
    float riseDelay;  // 踏んでから上がるまで
    float timer;      // 経過時間
    float riseSpeed;  // 上昇速度
	float timeOnUpRising; // 上昇床に乗っている時間
   
    
};

//回転する鉄球
struct RotatingBall {
    Vector2 center;   // 回転中心
    float radius;     // 回転半径
    float angle;      // 現在の角度（ラジアン）
    float angularSpeed; // 角速度（ラジアン/秒）
	float armLength;   // アームの長さ
    Vector2 GetBallPosition() const {
        return{
            center.x + armLength * cosf(angle),
            center.y + armLength * sinf(angle),
        };
    }
};

//回転する鉄球
struct MoveRotatingBall {
Vector2 center;   // 回転中心
    float radius;     // 回転半径
    float angle;      // 現在の角度（ラジアン）
	float angularSpeed; // 角速度（ラジアン/秒）
	float armLength;   // アームの長さ
    bool onGround = false;
    bool isFalling = false;
    bool reverse = false;
	float gravity = 1600.0f;
    Vector2 vel = { 0.0f,0.0f };//速度

    Vector2 GetBallPosition() const {//直行座標(x,y)
        return{
            center.x + armLength * cosf(angle),
            center.y + armLength * sinf(angle),
        };
	}
    Rectangle GetRect() const {
        Vector2 ballPos = GetBallPosition();
        return Rectangle{
            ballPos.x - radius,
            ballPos.y - radius,
            radius * 2,
            radius * 2
        };
	} 
};

//床を転がる鉄球
struct RollingBall {
    Vector2 center;
    float radius;
	float angle;//現在の角度
	float angularSpeed;//角速度

    Vector2 vel;
    float gravity = 1600.0f;
    bool onGround = false;
	float rollSpeed;//転がる速度
	float rollDerection; // 1.0f: 右回り, -1.0f: 左回り
  

    //カーソルボタントリガー待機
    bool waitForCursorClick = false;
    bool activated = true;

    // 当たり判定用
    Rectangle GetRect() const {
        return {
            center.x - radius,
            center.y - radius,
            radius * 2,
            radius * 2
        };
    }
};


//落下文字
struct FallingText {
    bool sensed = false;//感知したか
    bool isFalling = false;
    bool onGround = false;
    float fallDelay = 0.2f;
    float timer = 0.0f;
    float fallSpeed;
    int fontSize = 48;

    Rectangle rect{ 0,0,0,0 };//当たり判定
    Vector2 vel = { 0.0f,0.0f };//速度
    float gravity = 1600.0f;

    int padding = 0;//余白
    float tolerance = 300.0f;//感知範囲

    std::string text = "SAFE";
    Vector2 pos{ 0,0 };

    Font font = { 0 };

    void Init(Vector2 pos_, Font font_) {
        pos = pos_;
        font = font_;
        RecalcRect();
    }

    void RecalcRect() {
        if (font.texture.id == 0) {
            // フォントが未設定なら既定フォントで測る（安全策）
            Vector2 size = MeasureTextEx(GetFontDefault(), text.c_str(), (float)fontSize, 0.0f);
            rect = { pos.x, pos.y, size.x, size.y };
        }
        else {
            Vector2 size = MeasureTextEx(font, text.c_str(), (float)fontSize, 0.0f);
            rect = { pos.x, pos.y, size.x, size.y };
        }
    }

    Rectangle GetRect() const {
        // 修正：y を返すようにし、幅と高さに padding*2 を加える（左右上下対称）
        return Rectangle{
            rect.x - (float)padding,
            rect.y - (float)padding,
            rect.width + (float)padding * 2,
            rect.height + (float)padding * 2
        };
    }
};

// ボタンブロック（下から叩くとブロックが出現）
struct ButtonBlock {
    Rectangle buttonRect;       // ボタン本体（叩かれる側）
    std::vector<Rectangle> blockRects;      // 出現するブロックの位置
    bool activated = false;     // 叩かれたか
};

// 重力ブロック
struct GravityBlock {
    Rectangle rect;
};

//奥行きドア（手前と奥をつなぐ）
struct LayerDoor {
    Rectangle frontRect;   // 手前側のドア
    Rectangle backRect;    // 奥側のドア
};

//エレベーター（上キーで自動上昇、下キーで自動下降）
struct Elevator {
    Rectangle rect;          // エレベーター床
    float upperY;            // 上限Y座標
    float lowerY;            // 下限Y座標
    float speed;             // 移動速度
    bool goingUp = false;    // 上昇中か
    bool goingDown = false;  // 下降中か
    bool onPlayer = false;   // プレイヤーが乗っているか
};

// 触れると壊れるブロック
// 触れると壊れるブロック
struct TouchBreakBlock {
    Rectangle rect;
    bool triggered = false;  // プレイヤーが触れたか
    bool isBroken = false;   // 完全に壊れたか
    float timer = 0.0f;      // 触れてからの経過時間
    float breakDelay = 0.5f; // 壊れるまでの遅延（秒）
};

//下から触れただけで壊れるブロック
struct BottomBreakBlock {
    Rectangle rect;
    bool triggered = false;  // 触れたか
    float breakDelay = 0.5f; // 壊れるまでの遅延（秒）
    bool isBroken = false;   // 完全に壊れたか
    float timer = 0.0f;      // 経過時間

};
// 退出ドア
struct ExitDoor {
    Rectangle rect;
    int targetStage;
};

//コメントブロック（触れるとおじさんがコメントする／当たり判定なし）
struct CommentBlock {
    Rectangle rect;
    std::string message;     // 表示するコメント
    float duration = 3.0f;   // 表示秒数
    bool triggered = false;  // 一度発動したか
    float cooldown = 0.0f;   // 再発動までのクールダウン
};

//クレーン発射台
struct CraneLaunchPad {
    Rectangle rect;
    float launchVelY = -900.0f;
    bool triggered = false;
};

//クレーン状態
enum class CraneState { IDLE, DESCENDING, GRABBING, CARRYING, DONE };

//クレーン（天井ギミック）
struct Crane {
    Rectangle bodyRect;      // クレーンの本体（描画・判定用）
    float armLength = 0.0f;  // 現在のアーム長さ（天井からの距離）
    float maxArmLength = 200.0f;
    float detectRangeX = 80.0f;  // 横方向の感知範囲
    float descendSpeed = 300.0f;
    float carrySpeedX = 400.0f;
    int carryDir = 1;            // 1=右, -1=左
	float carryDist = 1000.0f;// 持ち去る距離
    float carriedX = 0.0f;
    float ceilingY = 0.0f;
    Rectangle playerRect = {};   // main.cppから毎フレーム更新するプレイヤー矩形
    CraneState state = CraneState::IDLE;
};

//ワープホール
struct Warp {
    Rectangle rect;
    Vector2 place;//指定の位置
    bool warped = false;
};

//一時的に出現する床
struct TempFloor {
    Rectangle rect;
    float timer = 0.0f;
    float showSec = 100.0f;
	bool visible = false;//表示されているか
};

struct TempFloorSwitch {
    Rectangle rect;
    int targetFloor = -1;
	bool oneShot = true;//一度だけ作動するか
	bool triggered = false;//一度踏まれたか
	bool hover = false;//カーソルが上にあるか
};

struct DecorArrow {
    Rectangle rect;
    float angleDeg = 0.0f;
};

struct DecoSprite {
	Rectangle rect;
	SpriteId spriteId = SpriteId::None;  // 描画に使う画像パーツ
	float rotation = 0.0f;
	bool flipX = false;
	bool flipY = false;
};


// トゲ描画関数（宣言）
void DrawSpikes(Rectangle h, float spikeW);

