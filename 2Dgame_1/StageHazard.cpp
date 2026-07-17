#include "StageHazard.h"
#include "StageSnapPuzzle.h"
#include "Stage.h"
#include <cmath>

// ================================================================
// StageHazard.cpp の役割
// ---------------------------------------------------------------
// ・危険ギミック（トゲ、追尾、回転球、落下文字など）の起動・更新・判定を実装する。
// ・当たり判定そのものと、ギミックの状態遷移を同ファイルで管理している。
// ================================================================

//===========================================
// 内部ヘルパー関数（static）
//===========================================

static bool CheckOverlapX(const Rectangle& player, const Rectangle& object) {
	//X方向が重なっているか
	float playerLeft = player.x;
	float playerRight = player.x + player.width;
	float objectLeft = object.x;
	float objectRight = object.x + object.width;

	bool overlapX = (playerRight > objectLeft) && (playerLeft < objectRight);

	return overlapX;
}

//とげの右側（またはdir=-1のとき左側）近傍にいるか判定
static bool CheckPlayerOnRightSide(const Rectangle& player, const Rectangle& hazard, float tolX, int dir = 1) {
	if (dir > 0) {
		float playerLeft = player.x;
		float hazardRight = hazard.x + hazard.width;
		// プレイヤーの左端がトゲの右端 − tolX 以上なら「右側近傍」と判断する
		return (playerLeft >= hazardRight - tolX);
	}
	else {
		float playerRight = player.x + player.width;
		float hazardLeft = hazard.x;
		// プレイヤーの右端がトゲの左端 + tolX 以下なら「左側近傍」と判断する
		return (playerRight <= hazardLeft + tolX);
	}
}

static bool CheckNearSameY(const Rectangle& player, const Rectangle& hazard, float tolerance) {
	float playerCenterY = player.y + player.height * 0.5f;
	float hazardCenterY = hazard.y + hazard.height * 0.5f;
	return fabsf(playerCenterY - hazardCenterY) <= tolerance;
}


static bool CheckOverlapY(const Rectangle& player, const Rectangle& hazard) {
	//Y方向が重なっているか
	float playerTop = player.y;
	float playerBottom = player.y + player.height;
	float hazardTop = hazard.y;
	float hazardBottom = hazard.y + hazard.height;
	bool overlapY = (playerBottom >= hazardTop);
	return overlapY;
}



//とげの右側近傍にいるか判定
static bool CheckOverlapNearRight(const Rectangle& player, const Rectangle& hazard) {
	float playerLeft = player.x;
	float playerRight = player.x + player.width;
	//とげの右端付近にいるか
	float deadLineLeft = hazard.x + hazard.width;
	float deadLineRight = deadLineLeft + 30.0f;//許容値
	bool nearRight = (playerRight >= deadLineLeft) && playerLeft <= deadLineRight;
	return nearRight;
}

//とげの上にプレイヤーが来たのか判定
static bool CheckMoveHazard(const Rectangle& player, const Rectangle& hazard, const Stage& stage) {
	bool overlapX = CheckOverlapX(player, hazard);

	if (!overlapX)return false;

	//上にいるを判定
	float playerBottom = player.y + player.height;
	float hazardtop = hazard.y;
	const float tolerance = 60.0f;//許容値
	bool nearTop = (playerBottom >= hazardtop - tolerance && playerBottom <= hazardtop + tolerance);

	bool fromAbobe = (player.y < hazardtop);

	return nearTop && fromAbobe;
}

//上にプレイヤーがきたのが判定（y軸延長）
static bool CheckMoveHazardExtY(const Rectangle& player, const Rectangle& hazard) {
	bool overlapX = CheckOverlapX(player, hazard);

	if (!overlapX)return false;

	//上にいるを判定
	float playerBottom = player.y + player.height;
	float hazardtop = hazard.y;

	const float tolerance = 300.0f;//許容値
	bool nearTop = (playerBottom >= hazardtop - tolerance && playerBottom <= hazardtop + tolerance);

	bool fromAbobe = (player.y < hazardtop);

	return nearTop && fromAbobe;
}

//下にプレイヤーが来たのか判定
static bool CheckPlayerNearBelow(const Rectangle& player, const Rectangle& obj, float tolerance)
{
	bool overlapX = CheckOverlapX(player, obj);
	if (!overlapX)return false;
	float playerTop = player.y;
	float objBottom = obj.y + obj.height;
	bool nearBelow = playerTop <= objBottom + tolerance;
	bool fromBelow = (playerTop > objBottom);
	return nearBelow && fromBelow;
}

static bool CheckPlayerNearX(const Rectangle& player, const Rectangle& obj, float tolerance)
{
	float playerLeft = player.x;
	float playerRight = player.x + player.width;
	float objLeft = obj.x;
	float objRight = obj.x + obj.width;
	bool nearX = (playerRight >= objLeft - tolerance) && (playerLeft <= objRight + tolerance);
	return nearX;
}

//とげが出てくる(triggerd中はせりあがる)
static void UpMoveHazard(Stage& stage, const Rectangle& player, float dt) {
	for (int i = 0; i < stage.moveCount; i++) {
		auto& mh = stage.moveHazards[i];

		//プレイヤーが動くとげに来たら起動
		if (!mh.triggerd && CheckMoveHazard(player, mh.rect, stage)) {
			mh.triggerd = true;
		}

		if (mh.triggerd) {
			float targetY = mh.startY - mh.raiseHeight;
			mh.rect.y -= mh.moveSpeed * dt;

			if (mh.rect.y < targetY) {
				mh.rect.y = targetY;
				mh.triggerd = false;
			}
		}
	}
}

//とげが出てくる(y軸延長)
static void UpMoveHazardExtY(Stage& stage, const Rectangle& player, float dt) {
	for (int i = 0; i < stage.moveExtYCount; i++) {
		auto& mhY = stage.moveHazardsExtY[i];

		//プレイヤーが動くとげに来たら起動
		if (!mhY.triggerd && CheckMoveHazardExtY(player, mhY.rect)) {
			mhY.triggerd = true;
		}

		if (mhY.triggerd) {
			float targetY = mhY.startY - mhY.raiseHeight;
			mhY.rect.y += mhY.moveSpeed * dt * mhY.dir;

			if (mhY.rect.y < targetY) {
				mhY.rect.y = targetY;
				mhY.triggerd = false;
			}
		}
	}
}
//とげが下に下がる
static void DownMoveHazardExtY(Stage& stage, const Rectangle& player, float dt) {
	for (int i = 0; i < stage.moveDownHazardExtYCount; i++) {
		auto& mhd = stage.moveDownHazardsExtY[i];

		//プレイヤーが動くとげに来たら起動
		if (!mhd.triggerd && CheckPlayerNearBelow(player, mhd.rect, mhd.tolelance)) {
			mhd.triggerd = true;
		}

		if (mhd.triggerd) {
			float targetY = mhd.startY + mhd.raiseHeight;
			mhd.rect.y += mhd.moveSpeed * dt * mhd.dir;

			if (mhd.rect.y > targetY) {
				mhd.rect.y = targetY;
				mhd.triggerd = false;
			}
		}
	}
}

//とげが出てくる(左右どちらか、dir対応、Yもあってるとき)
static void MoveHazarardX(Stage& stage, const Rectangle& player, float dt) {
	for (int i = 0; i < stage.moveHazardRightCount; i++) {
		auto& mhR = stage.moveHazardsRight[i];

		// プレイヤーがdir方向側の近くに来たら起動（dir=1:右側判定, dir=-1:左側判定）
		// toleranceX: X方向（dir側）の感知範囲, toleranceY: Y方向（高さのずれ）の感知範囲
		// 両方の条件を満たしたときのみ起動するため、範囲が「X∩Y」の矩形状に絞られる
		if (!mhR.triggerd && CheckPlayerNearX(player, mhR.rect, mhR.toleranceX)
			&& CheckNearSameY(player, mhR.rect, mhR.toleranceY)) {
			mhR.triggerd = true;
			mhR.timer = 0.0f;
		}

		if (mhR.triggerd) {
			mhR.timer += dt;
			if (mhR.timer > mhR.delay) {//遅延時間後に動き出す

				// dirが1なら右方向、-1なら左方向への最大移動先を計算（startX は初期左上座標）
				float targetX = mhR.startX + mhR.raiseWidth * mhR.dir;

				// dir方向へ移動
				mhR.rect.x += mhR.moveSpeed * dt * mhR.dir;

				// 目標を超えたら位置を固定して起動を終了する（無限追尾防止）
				bool reachedTarget = (mhR.dir > 0) ? (mhR.rect.x >= targetX) : (mhR.rect.x <= targetX);
				if (reachedTarget) {
					mhR.rect.x = targetX;
					mhR.triggerd = false;

				}
			}
		}
	}
}

//追尾するとげ
static void TrackingHazardMove(Stage& stage, const Rectangle& player, float dt) {
	for (int i = 0; i < stage.trackingHazardCount; i++) {
		auto& th = stage.trackingHazards[i];
		Vector2 playerCenter = { player.x + player.width / 2,
								player.y + player.height / 2 };
		Vector2 thCenter = { th.rect.x + th.rect.width / 2,
							th.rect.y + th.rect.height / 2 };
		//===========プレイヤーと追尾とげの中心間ベクトル===========
		float dx = playerCenter.x - thCenter.x;
		float dy = playerCenter.y - thCenter.y;

		float distance = sqrtf((playerCenter.x - thCenter.x) * (playerCenter.x - thCenter.x) +
			(playerCenter.y - thCenter.y) * (playerCenter.y - thCenter.y));


		//===========とげとスタート位置間ベクトル===========
		float dxStart = th.startPos.x - thCenter.x;
		float dyStart = th.startPos.y - thCenter.y;
		float distanceStart = sqrtf((th.startPos.x - thCenter.x) * (th.startPos.x - thCenter.x) +
			(th.startPos.y - thCenter.y) * (th.startPos.y - thCenter.y));


		//=====追尾中============
		if (distance <= th.trackingRange) {
			th.isTracking = true;
			th.returnToStart = false;
			if (distance >= 1.0f) {
				float dirX = dx / distance;//単位ベクトル化
				float dirY = dy / distance;
				float moveX = dirX * th.speed * dt;
				float moveY = dirY * th.speed * dt;
				th.rect.x += moveX;
				th.rect.y += moveY;
			}
		}//=====追尾範囲外==========
		else if (distance > th.trackingRange) {
			th.isTracking = false;
			th.returnToStart = true;

			if (distanceStart >= 1.0f) {
				float  dirStartX = dxStart / distanceStart;
				float  disSartY = dyStart / distanceStart;
				float moveStartX = dirStartX * th.speed * 0.5f * dt;
				float moveStartY = disSartY * th.speed * 0.5f * dt;
				th.rect.x += moveStartX;
				th.rect.y += moveStartY;
			}

		}
		else {
			//元の位置に戻る
			th.rect.x = th.startPos.x;
			th.rect.y = th.startPos.y;
			th.returnToStart = false;
		}

	}
}

static void StageSenseFallingTexts(Stage& stage, const Rectangle& player, float dt) {
	for (int i = 0; i < stage.fallingTextCount; i++) {
		auto& ft = stage.fallingTexts[i];

		// 既に感知済みならスキップ
		if (ft.sensed) continue;

		// 既存のヘルパー関数を使用
		// プレイヤーが落下文字の下にいるか判定
		if (CheckPlayerNearBelow(player, ft.rect, ft.tolerance)) {
			ft.sensed = true;
			ft.timer = 0.0f;
		}
	}
}
//===========================================
// 公開関数の実装
//===========================================

bool StageHitRisingSpike(const Stage& stage, const Rectangle& player) {
	for (int i = 0; i < stage.moveExtYCount; i++) {
		if (CheckCollisionRecs(player, stage.moveHazardsExtY[i].rect)) {
			return true;
		}
	}
	return false;
}

// 目的: ステージ内の全危険オブジェクト接触を一括判定する。
// 入力: stage と player矩形。
// 出力: どれか1つでも接触していれば true。
bool StageHitHazard(const Stage& stage, const Rectangle& player) {//const Stage& 本物を使うけど、変更は禁止
	for (int i = 0; i < stage.hazardCount; i++) {
		if (IsHazardDisabledBySnapPuzzle(stage, i)) continue;//スナップパズルで無効化されているとげは判定しない

		if (CheckCollisionRecs(player, stage.hazards[i])) {//接触判定
			return true;
		}
	}

	//動くとげをみる
	for (int i = 0; i < stage.moveCount; i++) {
		const auto& mh = stage.moveHazards[i];
		if (CheckCollisionRecs(player, mh.rect)) {
			return true;
		}
	}

	for (int i = 0; i < stage.deathBlockCount; i++) {
		if (CheckCollisionRecs(player, stage.deathBlocks[i])) {
			return true;
		}
	}

	//下に下がるとげY拡張をみる
	for (int i = 0; i < stage.moveDownHazardExtYCount; i++) {
		const auto& mdhY = stage.moveDownHazardsExtY[i];
		if (CheckCollisionRecs(player,mdhY.rect)){
			return true;
		}
	}

	//動くとげ右をみる(Yもあってるとき)
	for (int i = 0; i < stage.moveHazardRightCount; i++) {
		const auto& mhr = stage.moveHazardsRight[i];
		if (CheckCollisionRecs(player, mhr.rect)) {
			return true;
		}
	}

	//追尾するとげをみる
	for (int i = 0; i < stage.trackingHazardCount; i++) {
		const auto& th = stage.trackingHazards[i];
		if (CheckCollisionRecs(player, th.rect))
			return true;
		
	}

	//落下文字をみる
	for (int i = 0; i < stage.fallingTextCount; i++) {
		const auto& ft = stage.fallingTexts[i];
		if (CheckCollisionRecs(player, ft.GetRect()))			
		{
			return true;
		}
	}

	//回転する鉄球をみる
	for (int i = 0; i < stage.rotatingBallCount; i++) {
		const auto& rb = stage.rotatingBalls[i];
		Vector2 ballpos = rb.GetBallPosition();
		Vector2 playerCenter = { player.x + player.width / 2,
			player.y + player.height / 2 };
		float dx = ballpos.x - playerCenter.x;
		float dy = ballpos.y - playerCenter.y;
		Rectangle playerRect = { player.x,player.y,player.width,player.height };
		float distance = sqrtf(dx * dx + dy * dy);

		if (CheckCollisionCircleRec(ballpos, rb.radius, playerRect))
			return true;
	}

	//動く回転する鉄球をみる
	for (int i = 0; i < stage.moveRotatingBallCount; i++) {
		const auto& mrb = stage.moveRotatingBalls[i];
		Vector2 ballpos = mrb.GetBallPosition();

		if (CheckCollisionCircleRec(ballpos, mrb.radius, player))
			return true;
	}

	//転がる鉄球をみる
	for (int i = 0; i < stage.rollingBallCount; i++) {
		const auto& rb = stage.rollingBalls[i];

		if (CheckCollisionCircleRec(rb.center, rb.radius, player))
			return true;
	}

	return false;
}

//透明ブロックに当たったか
bool clearCheck(const Stage& stage, const Rectangle& player) {
	for (int i = 0; i < stage.clearsCount; i++) {
		if (CheckCollisionRecs(player, stage.clearBlocks[i].rect)) {
			return true;
		}
	}
	return false;
}

// 目的: 動的ハザードのトリガ検知と位置更新をまとめて処理する。
// 注意: 呼び出し順は体感や難易度に影響するため、並び替え時は要検証。
void HazardUpdate(Stage& stage, const Rectangle& player, float dt) {
	UpMoveHazard(stage, player, dt);//とげが出てくる
	UpMoveHazardExtY(stage, player, dt);//とげが出てくる(y軸延長)
	DownMoveHazardExtY(stage, player, dt);//とげが下に下がる(y軸延長)
	TrackingHazardMove(stage, player, dt);//追尾するとげ
	StageSenseFallingTexts(stage, player, dt);//落下文字の感知
	MoveHazarardX(stage, player, dt);//とげが左右どちらかに出てくる(Yもあってるとき)
}

void HazardReset(Stage& stage) {
	for (int i = 0; i < stage.fallingTextCount; i++) {
		stage.fallingTexts[i] = stage.fallingTextsInit[i];
	}
	for (int i = 0; i < stage.moveCount; i++) {
		stage.moveHazards[i] = stage.moveHazardsInit[i];
	}
	for (int i = 0; i < stage.moveExtYCount; i++) {
		stage.moveHazardsExtY[i] = stage.moveHazardsExtYInit[i];
	}
	for (int i = 0; i < stage.trackingHazardCount; i++)
	{
		stage.trackingHazards[i] = stage.trackingHazardsInit[i];
	}
	for (int i = 0; i < stage.moveDownHazardExtYCount; i++) {
		stage.moveDownHazardsExtY[i] = stage.moveDownHazardsExtYInit[i];
	}
	for (int i = 0; i < stage.moveHazardRightCount; i++) {
		stage.moveHazardsRight[i] = stage.moveHazardsRightInit[i];
	}
}