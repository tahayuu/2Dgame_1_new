#include "StageHazard.h"
#include "Stage.h"
#include <cmath>

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

//とげの右側近傍にいるか判定
static bool CheckPlayerOnRightSide(const Rectangle& player, const Rectangle& hazard, float tolX) {
	float playerLeft = player.x;
	float hazardRight = hazard.x + hazard.width;
	// プレイヤーの左端がトゲの右端 − tolX 以上なら「右側近傍」と判断する
	return (playerLeft >= hazardRight - tolX);
}

//Y座標が近いか判定
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


//とげが出てくる(右)
static void MoveHazarardNearRight(Stage& stage, const Rectangle& player, float dt) {
	for (int i = 0; i < stage.moveExtXCount; i++) {
		auto& mhX = stage.moveHazardsExtX[i];

		// プレイヤーがトゲの右付近に来たら起動
		if (!mhX.triggerd && CheckOverlapNearRight(player, mhX.rect)) {
			mhX.triggerd = true;
			mhX.timer = 0.0f;
		}

		if (mhX.triggerd) {
	
			// 右方向への最大移動先の右端を計算（startX は初期左上座標）
			float targetRight = mhX.startX + mhX.raiseWidth;

			// 右へ移動
			mhX.rect.x += mhX.moveSpeed * dt;

			// 現在の右端
			float currentRight = mhX.rect.x + mhX.rect.width;

			// 目標を超えたら位置を固定して起動を終了する（無限追尾防止）
			if (currentRight >= targetRight) {
				mhX.rect.x = targetRight - mhX.rect.width;
				mhX.triggerd = false;
			
			
			}
		}
	}
}

//とげが出てくる(右)Yもあってるとき
static void MoveHazarardRight(Stage& stage, const Rectangle& player, float dt) {
	for (int i = 0; i < stage.moveHazardRightCount; i++) {
		auto& mhR = stage.moveHazardsRight[i];

		// プレイヤーがトゲの右付近に来たら起動
		if (!mhR.triggerd && CheckPlayerOnRightSide(player, mhR.rect, 24.0f)
			&& CheckNearSameY(player, mhR.rect,40.0f)) {
			mhR.triggerd = true;
			mhR.timer = 0.0f;
		}

		if (mhR.triggerd) {
			mhR.timer += dt;
			if (mhR.timer > mhR.delay) {//遅延時間後に動き出す

				// 右方向への最大移動先の右端を計算（startX は初期左上座標）
				float targetRight = mhR.startX + mhR.raiseWidth;

				// 右へ移動
				mhR.rect.x += mhR.moveSpeed * dt;

				// 現在の右端
				float currentRight = mhR.rect.x + mhR.rect.width;

				// 目標を超えたら位置を固定して起動を終了する（無限追尾防止）
				if (currentRight >= targetRight) {
					mhR.rect.x = targetRight - mhR.rect.width;
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

// オブジェクトに当たったか
bool StageHitHazard(const Stage& stage, const Rectangle& player) {//const Stage& 本物を使うけど、変更は禁止
	for (int i = 0; i < stage.hazardCount; i++) {
		if (CheckCollisionRecs(player, stage.hazards[i])) {
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

	//動くとげX拡張をみる
	for (int i = 0; i < stage.moveExtXCount; i++) {
		const auto& mhX = stage.moveHazardsExtX[i];
		if (CheckCollisionRecs(player, mhX.rect)) {
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

void HazardUpdate(Stage& stage, const Rectangle& player, float dt) {
	UpMoveHazard(stage, player, dt);//とげが出てくる
	UpMoveHazardExtY(stage, player, dt);//とげが出てくる(y軸延長)
	DownMoveHazardExtY(stage, player, dt);//とげが下に下がる(y軸延長)
	MoveHazarardNearRight(stage, player, dt);//とげが右に出てくる
	TrackingHazardMove(stage, player, dt);//追尾するとげ
	StageSenseFallingTexts(stage, player, dt);//落下文字の感知
	MoveHazarardRight(stage, player, dt);//とげが右に出てくる(Yもあってるとき)
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
	for (int i = 0; i < stage.moveExtXCount; i++) {
		stage.moveHazardsExtX[i] = stage.moveHazardsExtXInit[i];
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