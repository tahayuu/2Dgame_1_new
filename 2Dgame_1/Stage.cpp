#include"Stage.h"
#include "raylib.h"
#include "StageTypes.h"
#include "StageInit_1.h"
#include "StageInit_2.h"
#include "StageInit_3.h"
#include "StageInit_4.h"
#include "GameObjects.h"
#include "StageEventChanger.h"
#include "StageGimmickSignal.h"
#include "StageSnapPuzzle.h"
#include "EnemyManager.h"
#include "ItemManager.h"
#include <cmath>

// ================================================================
// Stage.cpp の役割
// ---------------------------------------------------------------
// ・ステージ内ギミックの状態更新、初期化、リセット処理を実装する。
// ・描画(StageDraw)や当たり判定(StageCollision/StageHazard)と責務を分離している。
// ================================================================

// 目的: カーソル追従床の追従処理を更新する。
void UpdateCursorPlatform(Stage& stage, float dt, Camera2D camera) {
	Vector2 mouseScreen = GetMousePosition(); // 画面座標
	Vector2 mousePos = GetScreenToWorld2D(mouseScreen, camera); // ワールド座標に変換
	bool mouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
	for (int i = 0; i < stage.cursorPlatformCount; i++) {
		auto& cp = stage.cursorPlatforms[i];

		if (!cp.isActive) continue; // 非アクティブならスキップ

		bool mouseOnPlatform = CheckCollisionPointRec(mousePos, cp.rect);

		if (mouseDown && mouseOnPlatform && !cp.isDragging) {
			cp.isDragging = true;
		}
		if (!mouseDown) {
			cp.isDragging = false;
		}

		if (cp.isDragging) {
			Vector2 center = cp.GetCenter();
			Vector2 platformCenter = {
				cp.rect.x + cp.rect.width / 2,
				cp.rect.y + cp.rect.height / 2
			};//床の中心位置
			// マウス位置と床の中心位置の差分ベクトルを計算
			Vector2 dir = {
				mousePos.x - center.x,
				mousePos.y - center.y
				/*  x が正なら、カーソルは床の右側にある
					x が	負なら、カーソルは床の左側にある
					y が正なら、カーソルは床の下側にある
					y が負なら、カーソルは床の上側にある*/
			};
			float dis = sqrt(dir.x * dir.x + dir.y * dir.y);//距離を計算

			if (dis > 1.0f) {
				//正規化
				dir.x /= dis;
				dir.y /= dis;

				float moveDist = cp.followSpeed * dt;//移動距離 = 速度 × 時間

				if (moveDist > cp.maxDistance) {
					moveDist = cp.maxDistance;
				}

				cp.rect.x += dir.x * moveDist;
				cp.rect.y += dir.y * moveDist;
			}

		}
	}
}


static void ActivateRollingBallsByCursorBottom(Stage& stage, const CursorBottom& cb) {
	// RollingBallを1個起動するための処理
	// 同じ処理を何回も書かないためにラムダ式でまとめている
	auto ActivateOne = [](RollingBall& rb) {
		// 起動済み状態にする
		rb.activated = true;

		// 地面にいる扱いを外して、落下を開始できるようにする
		rb.onGround = false;

		// Y方向の速度を一度リセットする
		// 前の速度が残っていると不自然に動く可能性があるため
		rb.vel.y = 0.0f;

		// rollDerection がほぼ0なら、方向が未設定とみなして -1 にする
		if (fabsf(rb.rollDerection) < 0.001f) {
			rb.rollDerection = -1.0f;
		}
		else {
			// 正の値なら右方向、負の値なら左方向として 1 または -1 にそろえる
			rb.rollDerection = (rb.rollDerection > 0.0f) ? 1.0f : -1.0f;
		}
		};

	// 今回1個でもRollingBallを起動したかどうか
	bool activatedAny = false;

	// targetRollingBall が有効な番号なら、その1個だけを起動する
	if (cb.targetRollingBall >= 0 && cb.targetRollingBall < stage.rollingBallCount) {
		ActivateOne(stage.rollingBalls[cb.targetRollingBall]);
		activatedAny = true;
	}
	else {
		// targetRollingBall が無効なら、
		// waitForCursorClick が true のRollingBallを探して起動する
		for (int i = 0; i < stage.rollingBallCount; i++) {
			auto& rb = stage.rollingBalls[i];

			// カーソルクリック待ちのRollingBallだけ起動する
			if (rb.waitForCursorClick) {
				ActivateOne(rb);
				activatedAny = true;
			}
		}

		// ここまでで1つも起動していない場合は、
		// 念のためすべてのRollingBallを起動する
		if (!activatedAny) {
			for (int i = 0; i < stage.rollingBallCount; i++) {
				ActivateOne(stage.rollingBalls[i]);
			}
		}
	}
}

// 目的: カーソル床スイッチのクリック検知と連動起動を処理する。
void UpdateCursorBottom(Stage& stage, float dt, Camera2D camera) {
	Vector2 mouseScreen = GetMousePosition(); // 画面座標
	Vector2 mousePos = GetScreenToWorld2D(mouseScreen, camera); // ワールド座標に変換
	bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

	for (int i = 0; i < stage.cursorBottomCount; i++) {
		auto& cb = stage.cursorBottoms[i];
		bool mouseOnBottom = CheckCollisionPointRec(mousePos, cb.rect);

		// 見た目用ホバー状態
		cb.isActive = mouseOnBottom;

		// oneShot なら再クリック不可
		if (cb.oneShot && cb.triggered) continue;

		if (mouseClicked && mouseOnBottom) {
			cb.triggered = true;
			ActivateRollingBallsByCursorBottom(stage, cb);
		}

	}
}

//一時床ギミックの更新
static void UpdateTempFloorGimmick(Stage& stage, float dt, Camera2D camera,bool allowMouseInput) {
	Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
	bool clicked = allowMouseInput && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);//マウスクリックが有効な場合のみ、クリック判定を行う

	for (int i = 0; i < stage.tempFloorSwitchCount; i++) {
		auto& sw = stage.tempFloorSwitches[i];
		sw.hover = allowMouseInput && CheckCollisionPointRec(mouseWorld, sw.rect);


		if (sw.triggered && sw.oneShot) continue;

		if (clicked && sw.hover) {
			for (int f = 0; f < stage.tempFloorCount; f++) {
				auto& tf = stage.tempFloors[f];
				tf.visible = true;
				tf.timer = tf.showSec;
			}
			sw.triggered = true;
		}
	}

	bool anyVisible = false;//少なくとも一つの床が表示されているか
	for (int i = 0; i < stage.tempFloorCount; i++) {
		auto& tf = stage.tempFloors[i];
		if (!tf.visible) continue;

		tf.timer -= dt;
		if (tf.timer <= 0.0f) {
			tf.timer = 0.0f;
			tf.visible = false;
		}
		else {
			anyVisible = true;
		}
	}
	if (!anyVisible) {
		for (int i = 0; i < stage.tempFloorSwitchCount; i++) {
			auto& sw = stage.tempFloorSwitches[i];
				sw.triggered = false;
			
		}
	}

}

// 目的: ステージ内の動的ギミックを1フレーム分更新する。
// 入力: stage、経過時間dt、アイテム管理、カメラ。
// 出力: 各ギミックの位置・状態フラグが更新される。
// 注意: プレイヤー挙動に影響が大きいため、更新順変更時は要検証。
void StageUpdate(Stage& stage, float dt,ItemManager& itemManager, Camera2D camera) {

	const bool snapMouseConsumed =
		UpdateSnapPuzzles(stage, camera, dt);

	const bool distanceMouseConsumed =
		UpdateDistanceTriggerPieces(
			stage,
			camera,
			dt,
			!snapMouseConsumed
		);

	RebuildGimmickSignals(stage);

	const bool mouseConsumed =
		snapMouseConsumed ||
		distanceMouseConsumed;
	//一時床ギミックの更新
	UpdateTempFloorGimmick(stage, dt, camera, !mouseConsumed);


	//触れると壊れるブロック
	for (int i = 0; i < stage.touchBreakBlockCount; i++) {
		auto& tb = stage.touchBreakBlocks[i];
		if (tb.triggered && !tb.isBroken) {
			tb.timer += dt;
			if (tb.timer >= tb.breakDelay) {
				tb.isBroken = true;
			}
		}
	}

	for (int i = 0; i < stage.bottomBreakBlockCount; i++) {
		auto& bb = stage.bottomBreakBlocks[i];
		if (!bb.triggered || bb.isBroken) continue;
		bb.timer += dt;
		if (bb.timer >= bb.breakDelay) {  // 各ブロック固有の breakDelay を使用
			bb.isBroken = true;
		}
	}

	//落下床
	for (int i = 0; i < stage.fallingCount; i++) {
		auto& fp = stage.fallingPlatforms[i];//短縮名..毎回stage.fallingPlatforms[i]と書くのは面倒 &はコピー防ぐため
		if (fp.triggered && !fp.isfalling) {//「踏まれている かつ まだ落ちていない」
			fp.timer += dt;
			if (fp.timer >= fp.fallDelay) {
				fp.isfalling = true;
			}
		}

		if (fp.isfalling) {
			fp.rect.y += fp.fallSpeed * dt; //変化量 = 速度 × 時間
		}
	}
	if (!mouseConsumed) {
		//カーソル追従床
		UpdateCursorPlatform(stage, dt, camera);
     	//カーソルクリックギミック
		UpdateCursorBottom(stage, dt, camera);
	}


	//重力反転クールダウン
	if (stage.gravityCooldown > 0.0f) {
		stage.gravityCooldown -= dt;
	}
	//動く床X
	for (int i = 0; i < stage.movePlatformCountX; i++) {
		auto& mpx = stage.movePlatformsX[i];
		if (mpx.onplayer && !mpx.ismoved) {//プレイヤーが乗っている場合
			mpx.timer += dt;
			if (mpx.timer > mpx.delay) {
				mpx.ismoved = true;
			}
		}
		// 前フレームのX座標を保存（移動量計算用）
		mpx.prevX = mpx.rect.x;

		if (mpx.ismoved ) {
			mpx.rect.x += mpx.moveSpeed * dt;
			// 初期X座標 + 移動距離に達したら止める（相対移動として判定）
			if (mpx.rect.x >= mpx.initialX + mpx.moveDistance) {
				mpx.rect.x = mpx.initialX + mpx.moveDistance;
			}
		}
	}

	//動く床YXY
	for (int i = 0; i < stage.movePlatformCountYXY; i++) {
		auto& mpyxy = stage.movePlatformsYXY[i];
		mpyxy.prevX2 = mpyxy.rect.x;
		mpyxy.prevY1 = mpyxy.rect.y;
		mpyxy.prevY3 = mpyxy.rect.y;
		if (mpyxy.onplayer && !mpyxy.triggerd) {
			mpyxy.timer += dt;

			if (mpyxy.timer > mpyxy.delay) {
				mpyxy.triggerd = true;
				mpyxy.isMovingY1 = true;
				mpyxy.isMovingX2 = false;
				mpyxy.isMovingY3 = false;
				mpyxy.isMoving = true;
			}
		}
		// 最初のY移動
		if (mpyxy.isMovingY1 && mpyxy.triggerd && mpyxy.isMoving && mpyxy.onplayer) {
			mpyxy.rect.y += mpyxy.moveSpeedY1 * dt;

			float targetY = mpyxy.initialY + mpyxy.moveDistanceY1;

			if (mpyxy.rect.y >= targetY) {
				mpyxy.rect.y = targetY;

				mpyxy.isMovingY1 = false;
				mpyxy.isMovingX2 = true;
				mpyxy.isMovingY3 = false;
				mpyxy.isMoving = false;
			}
		}
		// 次のX移動
		else if (mpyxy.isMovingX2 && mpyxy.onplayer && mpyxy.triggerd) {
			mpyxy.rect.x += mpyxy.moveSpeedY1 * dt;

			float targetX = mpyxy.initialX + mpyxy.moveDistanceX2;

			if (mpyxy.rect.x >= targetX) {
				mpyxy.rect.x = targetX;

				mpyxy.isMovingY1 = false;
				mpyxy.isMovingX2 = false;
				mpyxy.isMovingY3 = true;
				mpyxy.isMoving = false;
			}
		}
		// 最後のY移動
		else if (mpyxy.isMovingY3 && mpyxy.onplayer && mpyxy.triggerd) {
			mpyxy.rect.y -= mpyxy.moveSpeedY1 * dt;

			float targetY = mpyxy.initialY - mpyxy.moveDistanceY3;

			if (mpyxy.rect.y <= targetY) {
				mpyxy.rect.y = targetY;

				mpyxy.timer = 0.0f;
				mpyxy.isMovingY1 = false;
				mpyxy.isMovingX2 = false;
				mpyxy.isMovingY3 = false;
				mpyxy.isMoving = false;
			}
		}
	}

	//当たると吹っ飛ばす壁
	for (int i = 0; i < stage.knockBackWallCount; i++) {
		auto& kbw = stage.knockBackWalls[i];

		// タッチ後、遅延時間待機中
		if (kbw.onTouch && !kbw.iswithdraw && !kbw.isRetrun && !kbw.twiceTouch) {
			kbw.timer += dt;
			if (kbw.timer >= kbw.delay) {
				kbw.timer = 0.0f;
				kbw.iswithdraw = true;
			}
		}

		// 状態2: 引っ込み中
		if (kbw.iswithdraw) {
			kbw.rect.x += kbw.knockBackVelocity.x * dt;
			float targetX = kbw.startX + kbw.withdraw;  // ← 相対位置で計算
			if (kbw.rect.x >= targetX) {
				kbw.rect.x = targetX;
				kbw.iswithdraw = false;
				kbw.isRetrun = true;
				kbw.stoptimer = 0.0f;
			}
		}

		// 状態3: 戻り中
		if (kbw.isRetrun) {
			kbw.stoptimer += dt;
			if (kbw.stoptimer >= kbw.limitStop) {
				kbw.rect.x -= kbw.knockBackVelocity.x * dt;
				if (kbw.rect.x <= kbw.startX - 50.0f) {
					kbw.twiceTouch = true;
					kbw.rect.x = kbw.startX;
					kbw.isRetrun = false;
					kbw.onTouch = false;
				}
			}
		}
	}

	//ジャンプ台
	for (int i = 0; i < stage.jumpPlatfromCount; i++) {
		auto& jp = stage.jumpPlatfroms[i];
		//タッチ後、遅延時間待機中
		if (jp.onTouch && !jp.isWithdraw && !jp.twiceTouch && !jp.isReturn) {
			jp.timer += dt;
			if (jp.timer >= jp.delay) {
				jp.isWithdraw = true;
				jp.timer = 0.0f;
			}
		}
		//状態2:引っ込み中
		if (jp.isWithdraw) {
			jp.rect.y += jp.jumpSpeed * dt;
			float targetY = jp.startY + jp.withdraw;
			if (jp.rect.y >= targetY) {
				jp.rect.y = targetY;
				jp.isReturn = true;
				jp.isWithdraw = false;
				jp.stopTimer = 0.0f;
			}
		}

		//状態3:戻り中
		if (jp.isReturn) {
			jp.stopTimer += dt;
			if (jp.stopTimer >= jp.limitStop) {
				jp.rect.y -= jp.jumpSpeed * dt;
				if (jp.rect.y <= jp.startY) {
					jp.rect.y = jp.startY;
					jp.isReturn = false;
					jp.twiceTouch = true;
					jp.onTouch = false;
				}
			}
		}


	}

	//人を発射する砲台
	for (int i = 0; i < stage.batteryHumanCount; i++) {
		auto& bh = stage.batteryHumans[i];
		float bhRight = bh.rect.x + bh.rect.width;
		float bhLeft = bh.rect.x;
		if (bh.onEnter) {
			bhRight += 1.0f;
			bh.onLeft = true;
			if (bh.onLeft) {
				bhLeft -= 1.0f;
				bh.onLeft = false;
			}
		}
	}

	//円軌道床
	for (int i = 0; i < stage.circlePlatformCount; i++) {
		auto& cp = stage.circlePlatforms[i];
		cp.prevPos = cp.getRectposition();
		cp.angle += cp.angleSpeed * dt;
		if (cp.angle > 2.0 * PI) {
			cp.angle -= 2.0f * PI;
		}
		else if (cp.angle < 0.0f) {
			cp.angle += 2.0f * PI;
		}
		Vector2 newPos = cp.getRectposition();
		cp.vel = { newPos.x - cp.prevPos.x,newPos.y - cp.prevPos.y };//１フレームの移動量
	}
	/*
	A["① prevPos = 現在位置を保存"] --> B["② angle を更新（回転）"]
	B --> C["③ newPos = 新しい位置を取得"]
	C --> D["④ vel = newPos - prevPos<br/>（移動量を計算）"]
	D --> E["MoveWithCirclePlatform で<br/>プレイヤーを vel 分動かす"]*/

	//回転する鉄球
	for (int i = 0; i < stage.rotatingBallCount; i++) {
		auto& rb = stage.rotatingBalls[i];
		rb.angle += rb.angularSpeed * dt;
		// 0〜2πの範囲に収める（角度の正規化）
		if (rb.angle > 2.0f * PI)rb.angle -= 2.0f * PI;
		if (rb.angle < 0.0f)rb.angle += 2.0f * PI;
	}

	//アームを中心に回転しする鉄球
	for (int i = 0; i < stage.moveRotatingBallCount; i++) {
		auto& mrb = stage.moveRotatingBalls[i];

		//回転処理
		if (!mrb.reverse) {
			mrb.angle += mrb.angularSpeed * dt;//角度更新 新しい角度 = 現在の角度 + 角速度 × 経過時間

		}
		//地面につくと逆回転
		else if (mrb.reverse) {
			mrb.angle += -mrb.angularSpeed * dt;//角度更新 新しい角度 = 現在の角度 + 角速度 × 経過時間

		}

		if (mrb.angle > 2.0f * PI)mrb.angle -= 2.0f * PI;//角度の正規化
		if (mrb.angle < 0.0f)mrb.angle += 2.0f * PI;//角度の正規化
	}

	//床の上を転がる鉄球
	for (int i = 0; i < stage.rollingBallCount; i++) {
		auto& rb = stage.rollingBalls[i];
		//waitForCursorClickがtrueのローリングボールは、activatedがtrueになるまでは動かない
		if (rb.waitForCursorClick && !rb.activated) {
			continue;
		}

		if (!rb.onGround) {
			rb.vel.y += rb.gravity * dt;//新しい速度 = 現在の速度 + 重力 × 時間
			rb.center.y += rb.vel.y * dt;//新しい位置 = 現在の位置 + 速度 × 時間
		}
		if (rb.onGround) {
			rb.center.x += rb.rollSpeed * rb.rollDerection * dt;//転がる処理
			//移動距離 = 速度 × 方向 × 時間
			float deltaAngle = (rb.rollSpeed * rb.rollDerection * dt) / rb.radius;
			rb.angle += deltaAngle;

			if (rb.angle > 2.0f * PI) rb.angle -= 2.0f * PI;
			if (rb.angle < 0.0f) rb.angle += 2.0f * PI;
		}
	}

	// 落下文字の更新
	for (int i = 0; i < stage.fallingTextCount; i++) {
		auto& ft = stage.fallingTexts[i];

		// 感知済みで落下前ならタイマーを進める
		if (ft.sensed && !ft.isFalling) {
			ft.timer += dt;
			if (ft.timer >= ft.fallDelay) {
				ft.isFalling = true;
				ft.onGround = false;
				ft.vel.y = 0.0f;
			}
		}

		// 落下処理（物理）
		if (ft.isFalling && !ft.onGround) {
			ft.vel.y += ft.gravity * dt;
			ft.pos.y += ft.vel.y * dt;
			ft.rect.y = ft.pos.y;
		}
	}

	//上昇床
	for (int i = 0; i < stage.upRisingCount; i++) {
		auto& ur = stage.upRisingPlatforms[i];
		if (ur.triggered && !ur.isrising) {
			ur.timer += dt;
			if (ur.timer >= ur.riseDelay) {
				ur.isrising = true;
			}
		}
		if (ur.isrising) {
			ur.rect.y -= ur.riseSpeed * dt;
		}
	}

	//往復上昇床
	for (int i = 0; i < stage.upDownCount; i++) {
		auto& ud = stage.upDouwnPlatforms[i];
		if (ud.isrising) {

			if (ud.goingUp) {
				ud.rect.y -= ud.riseSpeed * dt;
				if (ud.rect.y < ud.upperY) {
					ud.rect.y = ud.upperY;
					ud.timer += dt;
					if (ud.timer > ud.stopTimer) {
						ud.goingUp = false;
						ud.timer = 0.0f;
					}
				}
			}
			else {
				ud.rect.y += ud.fallSpeed * dt;
				if (ud.rect.y > ud.lowerY) {
					ud.rect.y = ud.lowerY;
					ud.timer += dt;
					if (ud.timer > ud.stopTimer) {
						ud.timer = 0.0f;
						ud.goingUp = true;
					}
				}

			}
		}
	}

	//スイッチによって動作する床
	for (int i = 0; i < stage.switchPlatformCount; i++) {
		auto& sp=stage.switchPlatforms[i];
		float startX = sp.rect.x;
		if(sp.switchedOn){
			sp.rect.x+=sp.speed.x * dt;
			sp.rect.y += sp.speed.y * dt;
			if (sp.rect.x >= sp.startX + sp.MaX) {//上限に達したら止める
				sp.rect.x = sp.startX + sp.MaX;//上限に達したら止める
				//sp.switchedOn = false;
			}
			if (sp.rect.y <= sp.startY + sp.MaxY) {//上限に達したら止める
				sp.rect.y = sp.startY + sp.MaxY;//上限に達したら止める
				//sp.switchedOn = false;
			}
		}
	}

	//アイテム出現ブロック
	for (int i = 0; i < stage.itemBlockCount; i++) {
		auto& ib = stage.itemBlocks[i];
		//アイテムが出現していて、まだ出現していない場合
		if (ib.isActive && !ib.hasSpawned) {
			ib.isActive = false;
			Vector2 spawnPos = { ib.rect.x + (ib.rect.width / 2.0f) - 16.0f,
				ib.rect.y + ib.rect.height - 16.0f };
			Vector2 initialVel = { 0.0f, -300.0f };
			itemManager.Spawn(ib.spawnType, spawnPos, initialVel, true);//アイテム出現
			ib.hasSpawned = true;
		}
	}
		//クレーン
		for (int i = 0; i < stage.craneCount; i++) {
			auto& cr = stage.cranes[i];

			switch (cr.state) {
			case CraneState::IDLE: {
				// クレーン本体の中心X
				float craneCenterX = cr.bodyRect.x + cr.bodyRect.width / 2;
				// プレイヤーの中心X・Y
				float playerCenterX = cr.playerRect.x + cr.playerRect.width / 2;
				float playerY       = cr.playerRect.y;

				// X方向：クレーンの真下（本体幅 + 少し余裕）に入っているか
				bool underCrane = fabsf(playerCenterX - craneCenterX) < (cr.bodyRect.width / 2 + 40.0f);

				// Y方向：クレーンの天井から detectRangeX px 以内の距離にいるか
				bool inRange = (playerY > cr.ceilingY) && 
				               (playerY < cr.ceilingY + cr.detectRangeX);

				if (underCrane && inRange) {
					cr.state = CraneState::DESCENDING;
				}
				break;
			}

			case CraneState::DESCENDING: {
				cr.armLength += cr.descendSpeed * dt;
				if (cr.armLength > cr.maxArmLength) {
					cr.armLength = cr.maxArmLength;
				}

				// フック先端の当たり判定
				float hookCX = cr.bodyRect.x + cr.bodyRect.width / 2;
				float hookY  = cr.ceilingY + cr.armLength;
				Rectangle hookRect = { hookCX - 10, hookY - 10, 20, 20 };

				// プレイヤーにフックが触れたら即掴む
				if (CheckCollisionRecs(hookRect, cr.playerRect)) {
					cr.state = CraneState::GRABBING;
				}
				// 最大まで伸びても当たらなければそのまま掴む
				else if (cr.armLength >= cr.maxArmLength) {
					cr.state = CraneState::GRABBING;
				}
				break;
			}

			case CraneState::GRABBING: {
				stage.playerGrabbedByCrane = true;
				cr.state = CraneState::CARRYING;
				cr.carriedX = 0.0f;
				break;
			}

			case CraneState::CARRYING: {
				cr.bodyRect.x += cr.carryDir * cr.carrySpeedX * dt;
				cr.carriedX += fabsf(cr.carryDir * cr.carrySpeedX * dt);

				if (cr.carriedX >= cr.carryDist) {//運ぶ距離を運び終えたら
					cr.state = CraneState::DONE;
					stage.playerCraneKill = true;
				}
				break;
			}

			}
		}
	

	//わかれる床
	for (int i = 0; i < stage.splitPlatformCount; i++) {
		auto& sp = stage.splitPlatforms[i];
		if (!sp.triggered) continue;
		sp.offsetX += sp.splitSpeed * dt;//開く量を定める
		if (sp.offsetX >= sp.maxGap) sp.offsetX = sp.maxGap;
		float halfW = sp.base.width / 2;
		sp.left = { sp.base.x - halfW * 2,sp.base.y,halfW,sp.base.height };
		sp.right = { sp.base.x + halfW * 2,sp.base.y,halfW,sp.base.height };

	}
}

		void UpdateMagnet(Stage& stage, Rectangle& player, Vector2& velocity, float dt,Camera2D camera) {
			Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
			bool mouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
			
			for (int i = 0; i < stage.magnetCount; i++) {
				auto& mg = stage.magnets[i];



				bool mouseOnMagnet = CheckCollisionPointRec(mousePos, mg.rect);
				/*if (mouseDown) {
					mg.triggerd = true;
				}*/
				if (mouseOnMagnet && mouseDown) {
					mg.isActive = true;
					mg.triggerd = true;
				}
				else if (!mouseDown) {
					mg.isActive = false;
					mg.triggerd = false;
				}

				if (mg.isActive) {
					Vector2 magnetCenter = mg.GetCenter();
					Vector2 playerCenter = {
						player.x + player.width / 2,
						player.y + player.height / 2
					};
					Vector2 dir = {
						magnetCenter.x - playerCenter.x,
						magnetCenter.y - playerCenter.y
					};

					float dis = sqrt(dir.x * dir.x + dir.y * dir.y);
					if (dis > 1.0f) {
						dir.x /= dis;
						dir.y /= dis;

						float movedis = mg.Force ;
						if (dis > mg.maxDistance) {
							mg.isActive = false;
						}
						else {
							velocity.x += dir.x * movedis;
							velocity.y += dir.y * movedis;
						}
					}
				}
			}
		}





		//ステージリセット
		// 目的: ステージ内の可変ギミックを初期状態へ戻す。
		void StageReset(Stage & stage) {
			for (int i = 0; i < stage.touchBreakBlockCount; i++) {
				stage.touchBreakBlocks[i] = stage.touchBreakBlocksInit[i];
			}
			for (int i = 0; i < stage.fallingCount; i++) {
				stage.fallingPlatforms[i] = stage.fallingPlatformsInit[i];
			}

			for (int i = 0; i < stage.knockBackWallCount; i++) {
				stage.knockBackWalls[i] = stage.knockBackWallsInit[i];
			}
			for (int i = 0; i < stage.upRisingCount; i++) {
				stage.upRisingPlatforms[i] = stage.upRisingPlatformsInit[i];
			}
			for (int i = 0; i < stage.upDownCount; i++) {
				stage.upDouwnPlatforms[i] = stage.upDouwnPlatformsInit[i];
			}
			stage.gravityCooldown = 0.0f;
			stage.gravityReversed = false;
			for (int i = 0; i < stage.gravityBlockCount; i++) {
				stage.gravityBlocks[i] = stage.gravityBlocksInit[i];
			}
			for (int i = 0; i < stage.movePlatformCountYXY; i++) {
				stage.movePlatformsYXY[i] = stage.movePlatformsYXYInit[i];
			}
			for (int i = 0; i < stage.rotatingBallCount; i++) {
				stage.rotatingBalls[i] = stage.rotatingBallsInit[i];
			}
			for (int i = 0; i < stage.moveRotatingBallCount; i++) {
				stage.moveRotatingBalls[i] = stage.moveRotatingBallsInit[i];
			}
			for (int i = 0; i < stage.rollingBallCount; i++) {
				stage.rollingBalls[i] = stage.rollingBallsInit[i];
			}
			for (int i = 0; i < stage.movePlatformCountX; i++) {
				stage.movePlatformsX[i] = stage.moveplatformsXInit[i];
			}
			for (int i = 0; i < stage.splitPlatformCount; i++) {
				auto& sp = stage.splitPlatforms[i];
				sp.base = sp.baseInit;
				sp.triggered = false;
				sp.offsetX = 0.0f;

				float halfW = sp.base.width / 2;
				sp.left = { sp.base.x,sp.base.y,halfW,sp.base.height };
				sp.right = { sp.base.x + halfW,sp.base.y,halfW,sp.base.height };
			}
			for (int i = 0; i < stage.movePlatformCountX; i++) {
				stage.moveplatformsXInit[i] = stage.movePlatformsX[i];
			}
			for (int i = 0; i < stage.touchBreakBlockCount; i++) {
				stage.touchBreakBlocks[i] = stage.touchBreakBlocksInit[i];
			}
			
			for (int i = 0; i < stage.itemBlockCount; i++) {
				stage.itemBlocks[i] = stage.itemBlocksInit[i];
			}
			for (int i = 0; i < stage.jumpPlatfromCount; i++) {
				stage.jumpPlatfroms[i] = stage.jumpPlatfromsInit[i];
			}
			for (int i = 0; i < stage.switchPlatformCount; i++) {
				stage.switchPlatforms[i] = stage.switchplatformsInit[i];
			}
			for (int i = 0; i < stage.breakableBlockCount; i++) {
				stage.breakableBlocks[i] = stage.breakableBlocksInit[i];
			}
			for (int i = 0; i < stage.clearsXCount; i++) {
				stage.clearBlocksX[i] = stage.clearBlocksXInit[i];
			}
			for (int i = 0; i < stage.buttonBlockCount; i++) {
				stage.buttonBlocks[i] = stage.buttonBlocksInit[i];
			}
			for (int i = 0; i < stage.deathBlockCount; i++) {
				stage.deathBlocks[i] = stage.deathBlocksInit[i];
			}
			for (int i = 0; i < stage.springCount; i++) {
				stage.springs[i] = stage.springsInit[i];
			}
			stage.heldSpringIndex = -1;  
			for (int i = 0; i < stage.elevatorCount; i++) {
				stage.elevators[i] = stage.elevatorsInit[i];
			}
			for (int i = 0; i < stage.bottomBreakBlockCount; i++) {
				stage.bottomBreakBlocks[i] = stage.bottomBreakBlocksInit[i];
			}
			for (int i = 0; i < stage.commentBlockCount; i++) {
				stage.commentBlocks[i] = stage.commentBlocksInit[i];
			}

			for (int i = 0; i < stage.cursorBottomCount; i++) {
				stage.cursorBottoms[i] = stage.cursorBottomsInit[i];
			}
			for (int i = 0; i < stage.deathBlockCount; i++) {
				stage.deathBlocks[i] = stage.deathBlocksInit[i];
			}
			stage.currentLayer = 0;
			for (int i = 0; i < stage.craneCount; i++) {
				stage.cranes[i] = stage.cranesInit[i];
			}
			for (int i = 0; i < stage.ojisanPunchAreaCount; i++) {
				stage.ojisanPunchTriggered[i] = false;
			}
			for (int i = 0; i < stage.tempFloorCount; i++) {
				stage.tempFloors[i] = stage.tempFloorsInit[i];
			}
			for (int i = 0; i < stage.tempFloorSwitchCount; i++) {
				stage.tempFloorSwitches[i] = stage.tempFloorSwitchesInit[i];
			}
			ResetSnapPuzzles(stage);
			ResetDistanceTriggerPieces(stage);

			ClearGimmickSignals(stage);
			ResetEventChangers(stage);
		}
	
		

		// 目的: ステージ切替前にカウント類を安全に初期化する。
		// 目的: ステージ切替前にカウント類を安全に初期化する。
		void StageClear(Stage& stage) {
			StageThemeUnload(stage.theme);
			stage.touchBreakBlockCount = 0;
			stage.springCount = 0;
			stage.heldSpringIndex = -1;
			stage.bottomBreakBlockCount = 0;
			stage.backPlatformCount = 0;
			stage.currentLayer = 0;
			stage.layerDoorCount = 0;
			stage.elevatorCount = 0;

			stage.cameraConfig = CameraConfig();
			stage.gravityBlockCount = 0;
			stage.gravityReversed = false;
			stage.gravityCooldown = 0.0f;
			stage.platformCount = 0;
			stage.buttonBlockCount = 0;
			stage.hazardCount = 0;
			stage.moveCount = 0;
			stage.moveExtYCount = 0;
			stage.fallingCount = 0;
			stage.upRisingCount = 0;
			stage.upDownCount = 0;
			stage.clearsCount = 0;
		stage.clearsXCount = 0;
			stage.splitPlatformCount = 0;
			stage.itemBlockCount = 0;
			stage.breakableBlockCount = 0;
			stage.knockBackWallCount = 0;
			stage.batteryHumanCount = 0;
			stage.jumpPlatfromCount = 0;
			stage.switchPlatformCount = 0;
			stage.icePlatformCount = 0;
			stage.cursorPlatformCount = 0;
			stage.magnetCount = 0;
			stage.moveUpPlatformCount = 0;
			stage.moveDownPlatformCount = 0;
			stage.circlePlatformCount = 0;
			stage.movePlatformCountX = 0;
			stage.moveHazardRightCount = 0;
			stage.moveDownHazardExtYCount = 0;
			stage.trackingHazardCount = 0;
			stage.rotatingBallCount = 0;
			stage.moveRotatingBallCount = 0;
			stage.rollingBallCount = 0;
			stage.movePlatformCountYXY = 0;
			stage.fallingTextCount = 0;
			stage.exitDoorCount = 0;
			stage.warpCount = 0;
			stage.hasRespawnPoint = false;
			stage.respawnPoint = { 100.0f, 500.0f };  // ← 追加
			stage.commentBlockCount = 0;
			stage.cursorPlatformCount = 0;
			stage.cursorBottomCount = 0;
			stage.magnetCount = 0;
			stage.deathBlockCount = 0;
			stage.spikeBouncerCount = 0;
			stage.craneCount = 0;
			stage.ojisanPunchAreaCount = 0;
			stage.tempFloorCount = 0;
			stage.tempFloorSwitchCount = 0;
			stage.decorArrowCount = 0;
			stage.decoSpriteCount = 0;
			stage.dragPieceCount = 0;
			stage.snapSlotCount = 0;
			stage.draggingSnapPieceIndex = -1;
			stage.snapMouseCaptured = false;
			stage.distanceTriggerPieceCount = 0;
			stage.draggingDistanceTriggerPieceIndex =-1;stage.distanceTriggerMouseCaptured =false;
            stage.eventChangerCount = 0;			
			stage.currentJumpMode = 0;
			stage.spriteInstanceCount = 0;
			for (int i = 0;i < MAX_HAZARDS;i++) {
            stage.hazardDisableSnapGroupIds[i] =0;
            stage.hazardDisableSignalIds[i] =0;
			}
			ClearGimmickSignals(stage);//ギミック信号の初期化
		}
// （新規追加）


void LoadSelectedStage(int selectStage, int& currentStage,
                       Stage& stage, EnemyManager& em, ItemManager& im,
                       /* stageDir等のパス情報 */ const std::string& stageBasePath)
{
    StageClear(stage);

    switch (selectStage) {
    case 0: StageInit_1(stage, em, im);     currentStage = 1;   break;
    case 1: StageInit_debug(stage, em, im); currentStage = 99;  break;
    case 2: StageInit_2(stage, em, im);     currentStage = 2;   break;
    case 3: StageInit_3(stage, em, im);     currentStage = 3;   break;
    // ...
    }

    im.saveItemsInit();
    em.saveEnemiesInit();
    em.RestorInitialEnemies();
    im.RestorInitialItems();
}
