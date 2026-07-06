#include "StageCollision.h"
#include "Stage.h"
#include"GameObjects.h"
#include <cmath>

//===========================================
// 内部ヘルパー関数（static）
//===========================================

 bool CheckOverlapX(const Rectangle& player, const Rectangle& object) {
	//X方向が重なっているか
	float playerLeft = player.x;
	float playerRight = player.x + player.width;
	float objectLeft = object.x;
	float objectRight = object.x + object.width;

	bool overlapX = (playerRight > objectLeft) && (playerLeft < objectRight);

	return overlapX;
}
 
 bool CheckOverlapXstrict(const Rectangle& player, const Rectangle& object, float strict) {
	 float playerLeft = player.x;
	 float playerRight = player.x + player.width;
	 float objectLeft = object.x;
	 float objectRight = object.x + object.width;
	 
	 bool overlapX = (playerRight > objectLeft + strict) && (playerLeft < objectRight - strict);
	 return overlapX;
 }


 static bool CheckOverlapY(const Rectangle& player, const Rectangle& hazard) {
	//Y方向が重なっているか
	float playerTop = player.y;
	float playerBottom = player.y + player.height;
	float hazardTop = hazard.y;
	float hazardBottom = hazard.y + hazard.height;
	bool overlapY = (playerBottom > hazardTop) && (playerTop < hazardBottom);
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


// prevSolid（前フレームの床位置）を使って「前フレームで床の上にいたか」を判定する。
static bool ResolveSolidXForMovingSolid(const Rectangle& solid, const Rectangle& prevSolid, Rectangle& player, Vector2& velocity, const Rectangle& prevPlayer) {
	if (!CheckCollisionRecs(player, solid)) return false;
	float prevBottom = prevPlayer.y + prevPlayer.height;
	const float epsion = 1.0f;
	// 前フレームのプレイヤー底が前フレームの床の上にあれば Y 判定と見做し X を押し戻さない
	if (prevBottom <= prevSolid.y + epsion) {
		return false;
	}
	if (velocity.x > 0) {
		player.x = solid.x - player.width;
		velocity.x = 0;
		return true;
	}
	else if (velocity.x < 0) {
		player.x = solid.x + solid.width;
		velocity.x = 0;
		return true;
	}
	return false;
}


static bool ResolveSolidX(const Rectangle& solid, Rectangle& player, Vector2& velocity, const Rectangle& prevPlayer) {


	if (!CheckCollisionRecs(player, solid)) return false;
	const float eps = 1.0f;


	const float prevLeft = prevPlayer.x;
	const float prevRight = prevPlayer.x + prevPlayer.width;
	const float prevTop = prevPlayer.y;
	const float prevBottom = prevPlayer.y + prevPlayer.height;

	const float solidLeft = solid.x;
	const float solidRight = solid.x + solid.width;
	const float solidTop = solid.y;
	const float solidBottom = solid.y + solid.height;

	// 前フレームでプレイヤーがブロックの上側にいた場合、
	// 今回の重なりは「上から落ちてきて床に乗った」可能性が高い。
	//
	// この場合にX方向で押し戻すと、
	// 着地しただけなのに左右へ瞬間移動する原因になる。
	//
	// そのため、X方向では解決せず、Y方向の当たり判定に任せる。
	if (prevBottom <= solidTop + eps) return false;


	if (prevTop >= solidBottom - eps) return false;

	// 現在のプレイヤーとブロックが、X方向にどれくらい重なっているかを求める
	//
	// fminf(player.x + player.width, solidRight)
	//   → プレイヤー右端とブロック右端のうち、左側にある方
	//
	// fmaxf(player.x, solidLeft)
	//   → プレイヤー左端とブロック左端のうち、右側にある方
	//
	// この差がX方向の重なり量になる
	const float overlapX =
		fmaxf(0.0f, fminf(player.x + player.width, solidRight) - fmaxf(player.x, solidLeft));

	// 現在のプレイヤーとブロックが、Y方向にどれくらい重なっているかを求める
	//
	// fminf(player.y + player.height, solidBottom)
	//   → プレイヤー下端とブロック下端のうち、上側にある方
	//
	// fmaxf(player.y, solidTop)
	//   → プレイヤー上端とブロック上端のうち、下側にある方
	//
	// この差がY方向の重なり量になる
	const float overlapY =
		fmaxf(0.0f, fminf(player.y + player.height, solidBottom) - fmaxf(player.y, solidTop));

	// 念のため、どちらかの重なりが0以下なら処理しない
	// CheckCollisionRecsを通っているので基本的には重なっているはずだが、
	// 誤差や境界ぴったりの状態を考えて安全にしている
	if (overlapX <= 0.0f || overlapY <= 0.0f) return false;

	// Y方向の重なりがとても薄い場合は、
	// ブロックの角を少しかすっただけの可能性が高い。
	//
	// その状態でX方向に押し戻すと、
	// 少し触れただけなのに横へワープしたように見えることがある。
	if (overlapY < 1.0f) return false;

	// overlapY が overlapX に比べてかなり小さい場合、
	// 食い込み方としては「横から壁にぶつかった」というより、
	// 「上面・下面に軽く引っかかった」可能性が高い。
	//
	// 例：
	// overlapX が大きく、overlapY が小さい
	// → 横には広く重なっているが、縦方向の食い込みが浅い
	// → 着地や天井衝突など、Y方向で解決した方が自然
	//
	// ここでX方向の押し戻しを止めることで、
	// 斜め上昇で天井に当たったときの横ワープを防ぎやすくする。
	if (overlapY <= overlapX * 0.6f) return false;

	// 前フレームでプレイヤーがブロックの左側にいて、
	// 今フレームでブロックの左側面を越えたかを判定する。
	//
	// prevRight <= solidLeft + eps
	//   → 前フレームではプレイヤー右端がブロック左端より左側にあった
	//
	// player.x + player.width >= solidLeft + eps
	//   → 今フレームではプレイヤー右端がブロック左端を越えている
	//
	// つまり、左から右へ進んでブロックの左側面に入ったという意味
	const bool crossedLeftFace =
		(prevRight <= solidLeft + eps) && (player.x + player.width >= solidLeft + eps);

	// 前フレームでプレイヤーがブロックの右側にいて、
	// 今フレームでブロックの右側面を越えたかを判定する。
	//
	// prevLeft >= solidRight - eps
	//   → 前フレームではプレイヤー左端がブロック右端より右側にあった
	//
	// player.x <= solidRight - eps
	//   → 今フレームではプレイヤー左端がブロック右端を越えている
	//
	// つまり、右から左へ進んでブロックの右側面に入ったという意味
	const bool crossedRightFace =
		(prevLeft >= solidRight - eps) && (player.x <= solidRight - eps);

	// プレイヤーが右に移動中で、ブロックの左側面を越えていた場合
	// → 明確にブロックの左側面へ衝突したと判断する
	if (velocity.x > 0.0f && crossedLeftFace) {

		// プレイヤーの右端がブロックの左端に合うように配置する
		// これでブロックの中にめり込んだ分を外へ戻す
		player.x = solidLeft - player.width;

		// 壁にぶつかったので横方向の速度を止める
		velocity.x = 0.0f;

		// X方向の当たり判定を解決した
		return true;
	}

	// プレイヤーが左に移動中で、ブロックの右側面を越えていた場合
	// → 明確にブロックの右側面へ衝突したと判断する
	if (velocity.x < 0.0f && crossedRightFace) {

		// プレイヤーの左端がブロックの右端に合うように配置する
		// これでブロックの中にめり込んだ分を外へ戻す
		player.x = solidRight;

		// 壁にぶつかったので横方向の速度を止める
		velocity.x = 0.0f;

		// X方向の当たり判定を解決した
		return true;
	}

	// 斜めから入った場合、前フレーム基準の crossed 判定が取れないことがある。
	//
	// 例：
	// ・速度が速く、1フレームで角を深く通過した
	// ・前フレーム時点ですでに少し近かった
	// ・ばねや移動床で斜めに強く押された
	//
	// そのような場合に何もしないと、
	// ブロックをすり抜ける可能性がある。
	//
	// そこで最後の救済処理として、
	// 横方向の速度が残っているなら、その方向に応じて最低限押し戻す。
	//
	// ただし、この処理は強めの補正なので、
	// 上の条件で「明らかにY方向の衝突」と判断された場合は、
	// ここまで来ないようにしている。
	if (velocity.x > 0.0f) {

		// 右に進んでいるので、ブロックの左側に戻す
		player.x = solidLeft - player.width;
		velocity.x = 0.0f;
		return true;
	}

	if (velocity.x < 0.0f) {

		// 左に進んでいるので、ブロックの右側に戻す
		player.x = solidRight;
		velocity.x = 0.0f;
		return true;
	}

	// 横方向の速度がない、またはX方向で解決する必要がない場合
	return false;
}

static float GetOverlapX(const Rectangle& a, const Rectangle& b) {
	const float left = fmaxf(a.x, b.x);//2つの矩形の左端のうち右側の座標
	const float right = fmin(a.x + a.width, b.x + b.width);//2つの矩形の右端のうち左側の座標);
	/*プレイヤー:      |------|
      ブロック:           |--------|
      重なり:             |---|             */
	return fmax(0.0f, right - left);
}

//プレイヤー用X衝突（重力反転対応）
static bool ResolveSolidXPlayer(const Rectangle& solid, Rectangle& player, Vector2& velocity, const Rectangle& prevPlayer, bool gravReversed) {
	if (!gravReversed) {
		return ResolveSolidX(solid, player, velocity, prevPlayer);
	}
	if (!CheckCollisionRecs(player, solid)) return false;

	float prevTop = prevPlayer.y;
	float prevBottom = prevPlayer.y + prevPlayer.height;
	const float epsilon = 1.0f;

	//反転時：ブロックの下にいた（天井に張り付き）→Y衝突と見なしXを押し戻さない
	if (prevTop >= solid.y + solid.height - epsilon) {
		return false;
	}
	//反転時：ブロックの上にいた（天井からジャンプで降下中）→Y衝突と見なしXを押し戻さない
	if (prevBottom <= solid.y + epsilon) {
		return false;
	}

	if (velocity.x > 0) {
		player.x = solid.x - player.width;
		velocity.x = 0;
		return true;
	}
	else if (velocity.x < 0) {
		player.x = solid.x + solid.width;
		velocity.x = 0;
		return true;
	}
	return false;
}

static bool ResolveSolidYForMovingSolid(const Rectangle& solid, const Rectangle& prevSolid, Rectangle& player, Vector2& velocity, const Rectangle& prevPlayer) {
	if (!CheckCollisionRecs(player, solid)) {
		// 床がフレーム内に移動して通過した領域（swept rect）を作成してチェックする
		float sweptTop = fminf(solid.y, prevSolid.y);
		float sweptHeight = solid.height + fabsf(prevSolid.y - solid.y);
		Rectangle swept = { solid.x, sweptTop, solid.width, sweptHeight };

		// プレイヤーがその移動軌跡と重なっていれば、床がプレイヤーを突き上げた可能性を検出する
		if (CheckCollisionRecs(player, swept)) {
			// 上向きに移動した（yが小さくなった）場合に限り、床が下からプレイヤーを押し上げたと判断する
			if (prevSolid.y > solid.y) {
				float prevTop = prevSolid.y;
				float currTop = solid.y;
				float prevPlayerBottom = prevPlayer.y + prevPlayer.height;
				// prevTop がプレイヤーの底より下（＝床がプレイヤーの下）で、current top がプレイヤーの底を越えていれば衝突
				if (prevTop > prevPlayerBottom && currTop <= prevPlayerBottom) {
					// プレイヤーを床の上にのせる
					player.y = currTop - player.height;
					velocity.y = 0;
					return true;
				}
			}
		}
		return false;
	}

	// 現フレームで重なっているなら通常の処理（頭当たり / 着地）
	const float eps = 2.0f;
	// 上向き（プレイヤーがジャンプ中）
	if (velocity.y < 0) {
		// 前フレームでプレイヤー底が prevSolid の上にあった (= 床の上に乗っていた) 場合は頭当たりを無視
		float prevBottom = prevPlayer.y + prevPlayer.height;
		if (prevBottom <= prevSolid.y + eps) {
			return false;
		}
		// そうでなければ頭当たりとして処理
		player.y = solid.y + solid.height;
		velocity.y = 0;
		return false;
	}
	// 下向き（落下中）で床に乗れた場合
	else if (velocity.y > 0) {
		player.y = solid.y - player.height;
		velocity.y = 0;
		return true;
	}
	return false;
}

static bool ResolveSolidY(const Rectangle& solid, Rectangle& player, Vector2& velocity, Rectangle& prev) {
	if (!CheckCollisionRecs(player, solid)) return false;

	const float eps = 1.0f;//誤差許容値
	const float nowOverlapX = GetOverlapX(player, solid);//現在のプレイヤーとブロックがX方向にどれくらい重なっているか
	const float minNeedOverlapX = fmaxf(4.0f, fminf(player.width, solid.width) * 0.25);//X方向の重なりがこれ以上小さい場合は、床に乗ったとはみなさない
	    //fminf = 小さい方の幅の25%以上重なっていないと、Y方向の衝突とは認めない
	    //最低4pxまたは小さい方の幅の25%のうち大きい方
	 
	
	// 上向き（プレイヤーがジャンプ中）の衝突判定
	if (velocity.y < 0) {
		const float solidBottom = solid.y + solid.height;
		const float prevTop = prev.y;
		const bool crossedFromBelow = (prevTop > solidBottom - eps) && (player.y < solidBottom - eps);
		// 前フレームで床の上にいた場合は頭当たりを無視

		if (!crossedFromBelow || nowOverlapX < minNeedOverlapX) {
			//下から境界をまたいでいないまたはX方向の重なりが少なすぎるなら、Y方向の当たり判定はしない
			return false;
		}
        
		//ここまでくれば頭をぶつけたときの押し戻し
		player.y = solidBottom;
		velocity.y = 0;
		return false;
	}
	// 下向き（落下中）で床に乗れる場合
	else if (velocity.y > 0) {
		const float solidTop = solid.y;
		const float prevBottom = prev.y + prev.height;
		const float nowBottom = player.y + player.height;

		const bool crossedFromAbove = (prevBottom <= solidTop + 2.0f) && (nowBottom > solidTop );

		if (!crossedFromAbove || nowOverlapX < minNeedOverlapX) {
			//上から境界をまたいでいないまたはX方向の重なりが少なすぎるなら、Y方向の当たり判定はしない
			return false;
		}

		player.y = solidTop - player.height;
		velocity.y = 0.0f;
		return true; 
	}
	return false;
}


//プレイヤー用Y衝突（重力反転対応）
static bool ResolveSolidYPlayer(const Rectangle& solid, Rectangle& player, Vector2& velocity, Rectangle& prev, bool gravReversed) {
	if (!gravReversed) {
		return ResolveSolidY(solid, player, velocity, prev);
	}
	if (!CheckCollisionRecs(player, solid)) return false;
	//反転時：上昇（落下）中→天井着地
	if (velocity.y < 0) {
		player.y = solid.y + solid.height;
		velocity.y = 0;
		return true; //天井に「着地」
	}
	//反転時：下降（ジャンプ）中→足がぶつかった
	else if (velocity.y > 0) {
		if (CheckCollisionRecs(prev, solid)) return false;
		player.y = solid.y - player.height;
		velocity.y = 0;
		return false;
	}
	return false;
}


//ｙ方向衝突解決（ジャンプ台用）
static bool JumpResolveSolidY(const Rectangle& solid, Rectangle& player, Vector2& velocity) {
	if (!CheckCollisionRecs(player, solid)) return false;


	if (velocity.y < 0) {//上昇中
		player.y = solid.y + solid.height;
		velocity.y = 0;//いったん０にすることで永遠に加算されない
		return true; //着地ではない
	}
	else if (velocity.y > 0) {//落下中
		player.y = solid.y - player.height;
		velocity.y = 0;
		return false;
	}
	return false;
}




//透明ブロック：下から当たったら出現判定をする
static void TrySpawnClearFromBelow(Stage& stage, const Rectangle& prev/*前フレームのプレイヤーの位置*/, Rectangle& player, Vector2& velocity) {
    for (int i = 0; i < stage.clearsCount; i++) {
        if (stage.clearBlocks[i].clearflag) continue; // 既に出現済みなら次へ
        Rectangle c = stage.clearBlocks[i].rect;
        bool hitNow = CheckCollisionRecs(player, c);//今プレイヤーと重なっているのか
        bool wasBelow = (prev.y >= c.y + c.height); //前のフレームではブロックの下にいたか？
        bool goingUp = (velocity.y < 0);            //上向きに動いているか？
        if (hitNow && wasBelow && goingUp) {//下からジャンプで突き上げた
            stage.clearBlocks[i].clearflag = true;
            player.y = c.y + c.height;
            velocity.y = 0;
            return;
        }
    }
}

//透明ブロック：横から当たったら出現判定
static void TrySpawnClearFromSide(Stage& stage, const Rectangle& prev/*前フレームのプレイヤーの位置*/, Rectangle& player, Vector2& velocity) {
	for (int i = 0; i < stage.clearsXCount; i++) {
		if (stage.clearBlocksX[i].clearflag) continue; // 既に出現済みなら次へ
		Rectangle c = stage.clearBlocksX[i].rect;
		bool hitNow = CheckCollisionRecs(player, c);//今プレイヤーと重なっているのか
		bool wasLeft = (prev.x + prev.width <= c.x); //前のフレームではブロックの左にいたか？
		bool wasRight = (prev.x >= c.x + c.width); //前のフレームではブロックの右にいたか？
		bool movingRight = (velocity.x > 0);            //右向きに動いているか？
		bool movingLeft = (velocity.x < 0);            //左向きに動いているか？
		if (hitNow && ((wasLeft && movingRight) || (wasRight && movingLeft))) {//横から突き上げた
			stage.clearBlocksX[i].clearflag = true;
			//押し戻し処理
			if (wasLeft && movingRight) {
				player.x = c.x - player.width;
			}
			else if (wasRight && movingLeft) {
				player.x = c.x + c.width;
			}
			velocity.x = 0;
			return;
		}
	}
}

//壊れるブロック：下から当たったら壊れる判定をする
static void TryBreakFromBelow(Stage& stage, const Rectangle& prev/*前フレームのプレイヤーの位置*/, Rectangle& player, Vector2& velocity) {
    for (int i = 0; i < stage.breakableBlockCount; i++) {
		if (stage.breakableBlocks[i].isBroken)continue; // 既に壊れ済みなら次へ
		Rectangle c = stage.breakableBlocks[i].rect;
		bool hitNow = CheckCollisionRecs(player, c);//今プレイヤーと重なっているのか
		bool wasBelow = (prev.y >= c.y + c.height); //前のフレームではブロックの下にいたか？
		bool goingUp = (velocity.y < 0);            //上向きに動いているか？
		if (hitNow && wasBelow && goingUp) {//下からジャンプで突き上げた
			stage.breakableBlocks[i].isBroken = true;
			player.y = c.y + c.height;
			velocity.y = 0;
			return;
		}
    }
}

//ボタンブロック：下から叩いたらブロック出現
static void TryActivateButtonBlock(Stage& stage, const Rectangle& prev, Rectangle& player, Vector2& velocity) {
    for (int i = 0; i < stage.buttonBlockCount; i++) {
        auto& bb = stage.buttonBlocks[i];
        if (bb.activated) continue;
        Rectangle b = bb.buttonRect;
        bool hitNow  = CheckCollisionRecs(player, b);
        bool wasBelow = (prev.y >= b.y + b.height);
        bool goingUp  = (velocity.y < 0);
        if (hitNow && wasBelow && goingUp) {
            bb.activated = true;
            player.y = b.y + b.height;
            velocity.y = 0;
            return;
        }
    }
}

//重力反転ブロック：下（or反転時は上）から叩いたら重力反転
//重力反転ブロック：下（or反転時は上）から叩いたら重力反転
static void TryActivateGravityBlock(Stage& stage, const Rectangle& prev, Rectangle& player, Vector2& velocity) {
	const float minActivateSpeed = 300.0f; // 上から叩く場合に必要な最低速度

	for (int i = 0; i < stage.gravityBlockCount; i++) {
		auto& gb = stage.gravityBlocks[i];
		Rectangle b = gb.rect;
		bool hitNow = CheckCollisionRecs(player, b);
		if (!hitNow) continue;

		if (!stage.gravityReversed) {
			//通常：下からジャンプで叩く（速度制限なし）
			bool wasBelow = (prev.y >= b.y + b.height);
			bool goingUp = (velocity.y < 0);
			if (wasBelow && goingUp) {
				stage.gravityReversed = true;
				player.y = b.y + b.height;
				velocity.y = 0;
				return;
			}
			//通常：上から落下で叩く（一定以上の速度が必要）
			bool wasAbove = (prev.y + prev.height <= b.y);
			bool goingDownFast = (velocity.y > minActivateSpeed);
			if (wasAbove && goingDownFast) {
				stage.gravityReversed = true;
				player.y = b.y - player.height;
				velocity.y = 0;
				return;
			}
		}
		else {
			//反転中：天井から「ジャンプ」（下向き）で叩く（速度制限なし）
			bool wasAbove = (prev.y + prev.height <= b.y);
			bool goingDown = (velocity.y > 0);
			if (wasAbove && goingDown) {
				stage.gravityReversed = false;
				player.y = b.y - player.height;
				velocity.y = 0;
				return;
			}
			//反転中：下から浮上で叩く（一定以上の速度が必要）
			bool wasBelow = (prev.y >= b.y + b.height);
			bool goingUpFast = (velocity.y < -minActivateSpeed);
			if (wasBelow && goingUpFast) {
				stage.gravityReversed = false;
				player.y = b.y + b.height;
				velocity.y = 0;
				return;
			}
		}
	}
}

//落下文字のｙ方向衝突解決
static bool ResolveFallingTextY(const Rectangle& solid, FallingText& ft, Rectangle& prev) {
	bool landed = ResolveSolidY(solid, ft.rect, ft.vel,prev);
	if (landed) {
		ft.onGround = true;
		ft.isFalling = false;     // 着地で止めたいなら
	}
	return landed;
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

//プレイヤーが移動床Xに乗っているときの処理プレイヤーも一緒に動かす
static void MoveWithPlayer(const Stage& stage, Rectangle& player, Vector2& velocity, float dt) {
	for (int i = 0; i < stage.movePlatformCountX; i++) {
		const auto& mpx = stage.movePlatformsX[i];
		if (mpx.onplayer) {  // フラグが立っていれば
			player.x += mpx.moveSpeed * dt;
		}
	}
}
//プレイヤーが往復上昇床に乗っているときの処理プレイヤーも一緒に動かす
static void MoveWithUpDownPlatform(const Stage& stage, Rectangle& player, Vector2& velocity, float dt) {
	for (int i = 0; i < stage.upDownCount; i++) {
		const auto& ud = stage.upDouwnPlatforms[i];
		if (ud.onUpDownPlatform && ud.isrising) {
			if (ud.goingUp && ud.rect.y > ud.upperY) {
				player.y -= ud.riseSpeed * dt;
			}
			else if (!ud.goingUp && ud.rect.y < ud.lowerY) {
				player.y += ud.fallSpeed * dt;
				velocity.y = 0;
			}

		}
	}
}
//プレイヤーが円軌道床に乗っているときの処理プレイヤーも一緒に動かす
static void MoveWithCirclePlatform(const Stage& stage, Rectangle& player, Vector2& velocity, float dt) {
	for (int i = 0; i < stage.circlePlatformCount; i++) {
		const auto& cp = stage.circlePlatforms[i];
		bool overlapX = CheckOverlapX(player, cp.GetRect());
		if (cp.onPlayer && overlapX) {
			player.x += cp.vel.x;
			player.y += cp.vel.y;
		}
	}
}

// 奥レイヤーの床を描画と同じスケールに縮小（中心基準）
static Rectangle ScaleRectCenter(const Rectangle& r, float scale) {
	float sw = r.width * scale;
	float sh = r.height * scale;
	return {
		r.x + (r.width - sw) / 2,
		r.y + (r.height - sh) / 2,
		sw, sh
	};
}

//下から触れただけで壊れるブロック
//ブロック下面がプレイヤー上部と重なった時点で自動的にトリガー
static void TryBreakFromBelowOnly(Stage& stage, const Rectangle& prev, Rectangle& player, Vector2& velocity) {
	for (int i = 0; i < stage.bottomBreakBlockCount; i++) {
		auto& bb = stage.bottomBreakBlocks[i];
		if (bb.isBroken || bb.triggered) continue;

		Rectangle c = bb.rect;

		// X方向の重なりがなければスキップ
		if (!CheckOverlapX(player, c)) continue;

		float blockBottom = c.y + c.height;

		// プレイヤーがブロックの下側にいるか
		// （player.y > c.y ＝ プレイヤー上面がブロック上面より下）
		bool playerIsBelow = (player.y > c.y);

		// ブロック下面がプレイヤーの体内に入っているか
		bool blockBottomInPlayer = (blockBottom >= player.y) && (blockBottom <= player.y + player.height);

		if (playerIsBelow && blockBottomInPlayer) {
			bb.triggered = true; // 崩壊開始（物理は ResolveSolidYPlayer に任せる）
		}
	}
}

//===========================================
// 公開関数の実装
//===========================================

//当たり判定をまとめて処理（ｘ方向）
void StageResolveX(Stage& stage, Rectangle& player, Vector2& velocity, float dt,Rectangle& prevPlayer) {
	TrySpawnClearFromSide(stage, prevPlayer, player, velocity);
	//普通床（現在のレイヤーのみ衝突）
	if (stage.currentLayer == 0) {
		for (int i = 0; i < stage.platformCount; i++) {
			ResolveSolidX(stage.platforms[i], player, velocity,prevPlayer);
		}
	}
	else {
		for (int i = 0; i < stage.backPlatformCount; i++) {
			Rectangle scaled = ScaleRectCenter(stage.backPlatforms[i], stage.BACK_LAYER_SCALE);
			ResolveSolidX(scaled, player, velocity, prevPlayer);
		}
	}

	// ===== 以下は手前レイヤー(layer==0)のときだけプレイヤーと衝突 =====
	if (stage.currentLayer == 0) {

	//エレベーター
	for (int i = 0; i < stage.elevatorCount; i++) {
		ResolveSolidXPlayer(stage.elevators[i].rect, player, velocity, prevPlayer, stage.gravityReversed);
	}

	//氷床 
	for (int i = 0; i < stage.icePlatformCount; i++) {
		ResolveSolidX(stage.icePlatforms[i].rect, player, velocity, prevPlayer);
	}

	//ボタンブロック
	for (int i = 0; i < stage.buttonBlockCount; i++) {
		ResolveSolidX(stage.buttonBlocks[i].buttonRect, player, velocity, prevPlayer);
		if (stage.buttonBlocks[i].activated) {
			for (const auto& br : stage.buttonBlocks[i].blockRects) {
				ResolveSolidX(br, player, velocity, prevPlayer);
			}
		}
	}

	//アイテムブロック
	for (int i = 0; i < stage.itemBlockCount; i++) {
		ResolveSolidX(stage.itemBlocks[i].rect, player, velocity, prevPlayer);
	}

	//カーソル追従床
	for (int i = 0; i < stage.cursorPlatformCount; i++) {
		if (stage.cursorPlatforms[i].isActive) {
			ResolveSolidX(stage.cursorPlatforms[i].rect, player, velocity, prevPlayer);
		}
	}
	//円軌道床
	for (int i = 0; i < stage.circlePlatformCount; i++){
		ResolveSolidX(stage.circlePlatforms[i].GetRect(),player,velocity, prevPlayer);
	} 

	//持ち運びジャンプ台
	for (int i = 0; i < stage.springCount; i++) {
		ResolveSolidX(stage.springs[i].rect, player, velocity, prevPlayer);
	}


	//吹っ飛ばし壁
	for (int i = 0; i < stage.knockBackWallCount; i++) {
		auto& kbw = stage.knockBackWalls[i];

		const float knockBackMargin = 50.0f;
		Rectangle extendedRect = {
			kbw.rect.x - knockBackMargin,
			kbw.rect.y,
			kbw.rect.width + 2 * knockBackMargin,
			kbw.rect.height
		};

		if (!CheckCollisionRecs(player, extendedRect)) continue;

		if (kbw.twiceTouch) {
			float playerCenterX = player.x + player.width * 0.5f;
			float wallCenterX = kbw.rect.x + kbw.rect.width * 0.5f;
			float dir = (playerCenterX < wallCenterX) ? -1.0f : 1.0f;

			velocity.x = dir * fabsf(kbw.knockBackVelocity.x);
			velocity.y = kbw.knockBackVelocity.y;
			kbw.twiceTouch = false;

			if (dir < 0.0f) {
				player.x = kbw.rect.x - player.width - knockBackMargin;
			}
			else {
				player.x = kbw.rect.x + kbw.rect.width + knockBackMargin;
			}

			stage.playerKnockedBack = true;
			continue;
		}
		else if (!kbw.onTouch && !kbw.iswithdraw && !kbw.isRetrun && !kbw.twiceTouch) {
			kbw.onTouch = true;
			kbw.timer = 0.0f;
			ResolveSolidX(kbw.rect, player, velocity, prevPlayer);
		}
		else {
			ResolveSolidX(kbw.rect, player, velocity, prevPlayer);
		}
	}

	// スパイク型トランポリン（横方向の衝突）
	for (int i = 0; i < stage.spikeBouncerCount; i++) {
		ResolveSolidX(stage.spikeBouncers[i].rect, player, velocity, prevPlayer);
	}

	//重力反転ブロック
	for (int i = 0; i < stage.gravityBlockCount; i++) {
		ResolveSolidXPlayer(stage.gravityBlocks[i].rect, player, velocity, prevPlayer, stage.gravityReversed);
	}

	//人を発射する砲台
	for (int i = 0; i < stage.batteryHumanCount; i++) {
		auto& bh = stage.batteryHumans[i];

		Vector2 bhCenter = { bh.rect.x + bh.rect.width / 2, bh.rect.y + bh.rect.height / 2 };

		if (CheckCollisionRecs(player, bh.rect) && !bh.onEnter && !bh.isReturn) {
			bh.onEnter = true;
			bh.timer = 0.0f;
			player.x = bhCenter.x - player.width / 2;
			player.y = bhCenter.y - player.height / 2;
			stage.playerInBattery = true;
		}
		else if (bh.onEnter) {
			player.x = bhCenter.x - player.width / 2;
			player.y = bhCenter.y - player.height / 2;
			stage.playerInBattery = true;
			bh.isReturn = true;
			bh.onEnter = false;
		}
		else if (bh.isReturn) {
			bh.timer += dt;
			if (bh.timer >= bh.delay) {
				player.y = bh.rect.y - player.height - 5.0f;
				velocity.x = bh.BatteryVelocity.x;
				velocity.y = bh.BatteryVelocity.y;
				stage.playerFired = true;
				stage.playerInBattery = false;
				bh.isReturn = false;
				bh.timer = 0.0f;
			}
			else {
				player.x = bhCenter.x - player.width / 2;
				player.y = bhCenter.y - player.height / 2;
				stage.playerInBattery = true;
			}
		}
	}


	//移動低下床
	for (int i = 0; i < stage.moveDownPlatformCount; i++) {
		ResolveSolidX(stage.moveDownPlatforms[i].rect,player,velocity, prevPlayer);
	}
	//移動上昇床
	for (int i = 0; i < stage.moveUpPlatformCount; i++) {
		ResolveSolidX(stage.moveUpPlatforms[i].rect,player,velocity, prevPlayer);
	}
    //往復上昇床
	for (int i = 0; i < stage.upDownCount; i++) {
		ResolveSolidX(stage.upDouwnPlatforms[i].rect,player,velocity, prevPlayer);
	}
	//落下床
	for (int i = 0; i < stage.fallingCount; i++) {
		ResolveSolidX(stage.fallingPlatforms[i].rect, player, velocity, prevPlayer);
	}

	//ジャンプ台
	for (int i = 0; i < stage.jumpPlatfromCount; i++) {
		ResolveSolidX(stage.jumpPlatfroms[i].rect, player, velocity, prevPlayer);
	}
	//上昇床
	for (int i = 0; i < stage.upRisingCount; i++) {
		auto& ur = stage.upRisingPlatforms[i];
		Rectangle prevSolid = ur.rect;
		if (ur.isrising) prevSolid.y += ur.riseSpeed * dt;
		ResolveSolidXForMovingSolid(ur.rect, prevSolid, player, velocity, prevPlayer);
	}

	//落下文字
	for (int i = 0; i < stage.fallingTextCount; i++) {
		ResolveSolidX(stage.fallingTexts[i].GetRect(), player, velocity, prevPlayer);
	}

	//移動床X
	for (int i = 0; i < stage.movePlatformCountX; i++) {
		ResolveSolidX(stage.movePlatformsX[i].rect, player, velocity, prevPlayer);
	}
	//壊れるブロック
	for (int i = 0; i < stage.breakableBlockCount; i++) {
		if (!stage.breakableBlocks[i].isBroken) {
			ResolveSolidX(stage.breakableBlocks[i].rect, player, velocity, prevPlayer);
		}
	}
	//透明ブロック
	for (int i = 0; i < stage.clearsCount; i++) {
		if (stage.clearBlocks[i].clearflag) {
			ResolveSolidX(stage.clearBlocks[i].rect, player, velocity, prevPlayer);
		}
	}
	for (int i = 0; i < stage.clearsXCount; i++) {
		if (stage.clearBlocksX[i].clearflag) {
			ResolveSolidX(stage.clearBlocksX[i].rect, player, velocity, prevPlayer);
		}

	}
	//スイッチによって動作する
	for (int i = 0; i < stage.switchPlatformCount; i++) {
		auto& sp = stage.switchPlatforms[i];
		if (CheckCollisionRecs(player, sp.SwitchRect)) {
		 sp.switchedOn = true;
		}
		else {
			sp.switchedOn = false;
		}

		if (sp.switchedOn) {
			ResolveSolidX(stage.switchPlatforms[i].rect, player, velocity, prevPlayer);
		}
	}

	//触れると壊れるブロック
	for (int i = 0; i < stage.touchBreakBlockCount; i++) {
		auto& tb = stage.touchBreakBlocks[i];
		if (tb.isBroken) continue;
		if (ResolveSolidXPlayer(tb.rect, player, velocity, prevPlayer, stage.gravityReversed)) {
			tb.triggered = true;
		}
	}

	//下から触れただけで壊れるブロック（X方向：横からは壊れず押し戻すのみ）
	for (int i = 0; i < stage.bottomBreakBlockCount; i++) {
		if (stage.bottomBreakBlocks[i].isBroken) continue;
		ResolveSolidXPlayer(stage.bottomBreakBlocks[i].rect, player, velocity, prevPlayer, stage.gravityReversed);
	}

	//わかれる床（X方向：普通のブロックと同じ衝突）
	for (int i = 0; i < stage.splitPlatformCount; i++) {
		const auto& sp = stage.splitPlatforms[i];
		if (!sp.triggered) {
			ResolveSolidX(sp.base, player, velocity, prevPlayer);
		}
		else {
			ResolveSolidX(sp.left,  player, velocity, prevPlayer);
			ResolveSolidX(sp.right, player, velocity, prevPlayer);
		}
	}

	} // ===== if (stage.currentLayer == 0) 終わり =====
	
}

//当たり判定をまとめて処理（ｙ方向）地面に乗っているなら true
bool StageResolveY(Stage& stage, const Rectangle& prevPlayer, Rectangle& player, Vector2& velocity, Rectangle& prev, float dt) {
	static float jumpBoostBuffer = 0.0f;
	const float jumpBoostBufferTime = 0.4f;

	if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
		jumpBoostBuffer = jumpBoostBufferTime;
	}
	else if (jumpBoostBuffer > 0.0f) {
		jumpBoostBuffer -= dt;
	}
	bool jumpBoostBuffered = (jumpBoostBuffer > 0.0f);// ジャンプブーストが有効な状態か

    TrySpawnClearFromBelow(stage, prevPlayer, player, velocity);
    TryBreakFromBelow(stage, prevPlayer, player, velocity);
	TryBreakFromBelowOnly(stage, prevPlayer, player, velocity);
    TryActivateButtonBlock(stage, prevPlayer, player, velocity); 
	TryActivateGravityBlock(stage, prevPlayer, player, velocity);
    bool onGround = false;
	bool onTop = false; //上に当たったら true にする

	//触れると壊れるブロック
	for (int i = 0; i < stage.touchBreakBlockCount; i++) {
		auto& tb = stage.touchBreakBlocks[i];
		if (tb.isBroken) continue;
		if (ResolveSolidYPlayer(tb.rect, player, velocity, prev, stage.gravityReversed)) {
			onGround = true;
			tb.triggered = true; // 上に乗っても崩壊開始
		}
		// 下からジャンプで当たった場合も検知
		if (!tb.triggered && CheckCollisionRecs(player, tb.rect)) {
			tb.triggered = true;
		}
	}

	//普通床（現在のレイヤーのみ衝突）
	if (stage.currentLayer == 0) {
		for (int i = 0; i < stage.platformCount; i++) {
			if (ResolveSolidYPlayer(stage.platforms[i], player, velocity, prev, stage.gravityReversed)) {
				onGround = true;
			}
		}
	}
	else {
		for (int i = 0; i < stage.backPlatformCount; i++) {
			Rectangle scaled = ScaleRectCenter(stage.backPlatforms[i], stage.BACK_LAYER_SCALE);
			if (ResolveSolidYPlayer(scaled, player, velocity, prev, stage.gravityReversed)) {
				onGround = true;
			}
		}
	}


	// ===== 以下は手前レイヤー(layer==0)のときだけプレイヤーと衝突 =====
	if (stage.currentLayer == 0) {

	//エレベーター
	for (int i = 0; i < stage.elevatorCount; i++) {
		bool landed = ResolveSolidYPlayer(stage.elevators[i].rect, player, velocity, prev, stage.gravityReversed);
		if (landed) {
		 onGround = true;
		 stage.elevators[i].onPlayer = true;
		}
	}

	//重力反転ブロック
	for (int i = 0; i < stage.gravityBlockCount; i++) {
		if (ResolveSolidYPlayer(stage.gravityBlocks[i].rect, player, velocity, prev, stage.gravityReversed)) {
			onGround = true;
		}																								
	}

	// ボタンブロック本体 + 出現済みブロック
	for (int i = 0; i < stage.buttonBlockCount; i++) {
		if (ResolveSolidYPlayer(stage.buttonBlocks[i].buttonRect, player, velocity, prev, stage.gravityReversed)) {
			onGround = true;
		}
		if (stage.buttonBlocks[i].activated) {
			for (const auto& br : stage.buttonBlocks[i].blockRects) {
				if (ResolveSolidYPlayer(br, player, velocity, prev, stage.gravityReversed)) {
					onGround = true;
				}
			}
		}
	}
	//アイテムブロック
	for (int i = 0; i < stage.itemBlockCount; i++) {
		auto& ib = stage.itemBlocks[i];
		Rectangle b = ib.rect;

		bool hitNow = CheckCollisionRecs(player, b);
		bool wasBelow = (prevPlayer.y >= b.y + b.height);
		bool goingUp = (velocity.y < 0.0f);

		if (hitNow && wasBelow && goingUp) {
			ib.isActive = true;
			ib.hasSpawned = false;
			player.y = b.y + b.height;
			velocity.y = 0.0f;
		}
		else {
			if (ResolveSolidYPlayer(b, player, velocity, prev, stage.gravityReversed)) {
			 onGround = true;
			}
		}
	}

	//カーソル追従床
	for (int i = 0; i < stage.cursorPlatformCount; i++) {
		if (stage.cursorPlatforms[i].isActive) {
			if (ResolveSolidYPlayer(stage.cursorPlatforms[i].rect, player, velocity, prev, stage.gravityReversed)) {
				onGround = true;
			}
		}
	}
	//氷床
	for (int i = 0; i < stage.icePlatformCount; i++) {
		if(ResolveSolidYPlayer(stage.icePlatforms[i].rect,player,velocity, prev, stage.gravityReversed)){
			onGround=true;
		}
	}
	//吹っ飛ばし壁
	for (int i = 0; i < stage.knockBackWallCount; i++) {
		if(ResolveSolidYPlayer(stage.knockBackWalls[i].rect,player,velocity, prev, stage.gravityReversed)){
			onGround=true;
		}
	}

	//移動低下床
	for (int i = 0; i < stage.moveDownPlatformCount; i++) {
		if(ResolveSolidYPlayer(stage.moveDownPlatforms[i].rect,player,velocity, prev, stage.gravityReversed)){
			onGround=true;
		}
	}

	//移動上昇床
	for (int i = 0; i < stage.moveUpPlatformCount; i++) {
		if(ResolveSolidYPlayer(stage.moveUpPlatforms[i].rect,player,velocity, prev, stage.gravityReversed)){
			onGround=true;
		}
	}

	//ジャンプ台
	for (int i = 0; i < stage.jumpPlatfromCount; i++) {
		bool landed = ResolveSolidYPlayer(stage.jumpPlatfroms[i].rect, player, velocity, prev, stage.gravityReversed);
		auto& jp = stage.jumpPlatfroms[i];
		if (!landed) continue;
		if (jp.twiceTouch) {
			velocity.y = jp.jumpVelocity.y;
			velocity.x = jp.jumpVelocity.x;
			jp.twiceTouch = false;
			player.y = jp.rect.y - player.height;
			stage.playerJumped = true;
			onGround = false;
			continue;
		}
		else if (landed && !jp.onTouch && !jp.isReturn && !jp.isWithdraw) {
			jp.onTouch = true;
			onGround = true;
			jp.timer = 0.0f;
		}
		else {
			onGround =true;
		}
	}

	//スパイク型トランポリン
	for (int i = 0; i < stage.spikeBouncerCount; i++) {
		auto& sb = stage.spikeBouncers[i];
		if (CheckCollisionRecs(player, sb.rect) && velocity.y > 0) {
			player.y = sb.rect.y - player.height;
			velocity.y = sb.bounceVelosity.y;
			velocity.x = sb.bounceVelosity.x;
			onGround = true;
		}
	}
	for (int i = 0; i < stage.spikeBouncerCount; i++) {
		ResolveSolidX(stage.spikeBouncers[i].rect, player, velocity, prevPlayer);
	}

	//プレイヤーが持つばね
	if (stage.heldSpringIndex < 0 || stage.heldSpringIndex >= stage.springCount) {
		//ばねと衝突判定
		for (int i = 0; i < stage.springCount; i++){
			auto& sp = stage.springs[i];
			if (!sp.isActive) continue;

			// プレイヤーの足元がばねの上面と重なっているか
			if (CheckCollisionRecs(player, sp.rect) && velocity.y > 0) {
				player.y = sp.rect.y - player.height;// プレイヤーの足元をばねの上面に合わせる
				bool tuch = player.y + player.height - sp.rect.y < 10.0f; // 足元が近いか
				const float baseBounceRate = 0.85f;   // 通常は弱め
				const float jumpBoostRate = 1.35f;   // ジャンプ時だけ強め

				// 通常値
				float baseBounce = sp.bounceVelocity.y * baseBounceRate;
				// ブースト値
				float boostedBounce = sp.bounceVelocity.y * jumpBoostRate;

				// jumpBoostBuffered を使う（StageResolveY の冒頭で作ったバッファ判定）
				if (jumpBoostBuffered && tuch) {
					velocity.y = boostedBounce;
				}
				else {
					velocity.y = baseBounce;
				}
				velocity.x = sp.bounceVelocity.x;
				onGround = true;
			}
		
		}
	}

	//ばねに乗った状態でローリングボールが近づいてきたときの処理
	for (int b = 0; b < stage.rollingBallCount; b++) {
		auto& rb = stage.rollingBalls[b];
		for (int i = 0; i < stage.springCount; i++) {
			auto& sp = stage.springs[i];
			if (!sp.isActive) continue;

			Rectangle ballRect = rb.GetRect();
			if (!CheckCollisionRecs(ballRect, sp.rect)) continue;

			float ballLeft = ballRect.x;
			float ballRight = ballRect.x + ballRect.width;
			float ballTop = ballRect.y;
			float ballBottom = ballRect.y + ballRect.height;

			float springLeft = sp.rect.x;
			float springRight = sp.rect.x + sp.rect.width;
			float springTop = sp.rect.y;
			float springBottom = sp.rect.y + sp.rect.height;

			float overlapX = fminf(ballRight, springRight) - fmaxf(ballLeft, springLeft);
			float overlapY = fminf(ballBottom, springBottom) - fmaxf(ballTop, springTop);
			// 重なりがある場合、より小さい方の軸で衝突を解決
			if (overlapX < overlapY) {
				float dir = 0.0f;
				if (fabsf(rb.vel.x) > 0.01f) {
					dir = (rb.vel.x > 0.0f) ? 1.0f : -1.0f;
				}
				else {
					float ballCx = ballRect.x + ballRect.width * 0.5f;// ボールの中心X
					float springCx = sp.rect.x + sp.rect.width * 0.5f;// ばねの中心X
					dir = (ballCx < springCx) ? -1.0f : 1.0f;
				}
				if (dir > 0.0f) {
					rb.center.x = springRight + rb.radius + 0.1f;
				}
				else {
					rb.center.x = springLeft - rb.radius - 0.1f;
				}

				const float sideMinSpeed = 220.0f;
				rb.vel.x = dir * fmaxf(fabsf(rb.vel.x), sideMinSpeed);

				rb.vel.y = sp.bounceVelocity.y * 0.65f;
			}
			else if (ballBottom >= springTop && rb.vel.y > 0.0f) {
				rb.center.y = springTop - rb.radius;
				rb.vel.y = sp.bounceVelocity.y;

				// 横速度を保持（ほぼ0のときだけばねの横速度を使う）
				if (fabsf(rb.vel.x) < 1.0f) {
				 rb.vel.x = sp.bounceVelocity.x;
				}
			}
		}
	}

	//落下床
	for (int i = 0; i < stage.fallingCount; i++) {
		auto& fp = stage.fallingPlatforms[i];
		bool landed = ResolveSolidYPlayer(fp.rect, player, velocity, prev, stage.gravityReversed);
		if (landed) {
			onGround = true;
			// detectMode: 0=両方, 1=プレイヤーのみ, 2=ボールのみ
			if (!fp.triggered && (fp.detectMode == 0 || fp.detectMode == 1)) {
				fp.triggered = true;
				fp.timer = 0.0f;
			}
		}
	}

	// --- 落下文字（プレイヤーとの衝突） ---
	for (int i = 0; i < stage.fallingTextCount; i++) {
		auto& ft = stage.fallingTexts[i];
		Rectangle r = ft.GetRect();

		if (!CheckCollisionRecs(player, r)) {
		}
		else {
			bool playerWasAbove = (prevPlayer.y + prevPlayer.height <= r.y + 0.1f);

			if (playerWasAbove) {
				if (ResolveSolidYPlayer(r, player, velocity, prev, stage.gravityReversed)) {
					onGround = true;
				}
			}
			else if (ft.isFalling && ft.vel.y > 0.0f) {
				ft.isFalling = false;
				ft.onGround = true;
				ft.vel.y = 0.0f;
				ft.pos.y = player.y - ft.rect.height;
				ft.rect.y = ft.pos.y;
			}
			else {
				if (ResolveSolidYPlayer(r, player, velocity, prev, stage.gravityReversed)) onGround = true;
			}
		}
	}

	//上昇床
	for (int i = 0; i < stage.upRisingCount; i++) {
		auto& ur = stage.upRisingPlatforms[i];
		Rectangle prevSolid = ur.rect;
		if (ur.isrising) prevSolid.y += ur.riseSpeed * dt;
		bool landed = ResolveSolidYForMovingSolid(ur.rect, prevSolid, player, velocity, prev);
		if (landed) {
			onGround = true;
			stage.playerOnUpRising = true;
			ur.timeOnUpRising = 0.0f; 
			if (!ur.triggered && onGround) {
				ur.triggered = true;
				ur.timer = 0.0f;
			}
		}
		else if (stage.playerOnUpRising) {
			ur.timeOnUpRising += dt;
			if (ur.timeOnUpRising >= 1.5f) {
				ur.timeOnUpRising = 0.0f;
				stage.playerOnUpRising = false;
			}
		}
	}

	//円軌道床
	for (int i = 0; i < stage.circlePlatformCount; i++) {
		bool landed = ResolveSolidYPlayer(stage.circlePlatforms[i].GetRect(), player, velocity, prev, stage.gravityReversed);
		auto& cp = stage.circlePlatforms[i];
		if (landed) {
		 onGround = true;
		 cp.onPlayer = true;
		}
	}

	// 往復上昇床のフラグをリセット
	for (int i = 0; i < stage.upDownCount; i++) {
		stage.upDouwnPlatforms[i].onUpDownPlatform = false;
	}
	//往復上昇床
	for (int i = 0; i < stage.upDownCount; i++) {
		auto& ud = stage.upDouwnPlatforms[i];
		Rectangle prevSolid = ud.rect;
		if (ud.goingUp || ud.isrising) prevSolid.y += ud.riseSpeed * dt;
		bool landed = ResolveSolidYForMovingSolid(ud.rect, prevSolid, player, velocity, prev);
		if(landed){
			onGround=true;
			ud.onUpDownPlatform = true;
		}
		if (!ud.isrising) {
			ud.isrising = true;
			ud.timer = 0.0f;
		}
	}

	// 移動床のフラグをリセット
	for (int i = 0; i < stage.movePlatformCountX; i++) {
		stage.movePlatformsX[i].onplayer = false;
	}

	//移動床X
	for (int i = 0; i < stage.movePlatformCountX; i++) {
		bool landed = ResolveSolidYPlayer(stage.movePlatformsX[i].rect, player, velocity, prev, stage.gravityReversed);
		auto& mpx = stage.movePlatformsX[i];
		if (landed) {
		 onGround = true;
		 mpx.onplayer = true;
		}
	 if (landed && !mpx.triggerd) {
		 mpx.triggerd = true;
		 mpx.timer = 0.0f;
	 }
	}

	//わかれる床
	for (int i = 0; i < stage.splitPlatformCount; i++) {
		auto& sp = stage.splitPlatforms[i];
		if (!sp.triggered) {
			bool landed = ResolveSolidYPlayer(sp.base, player, velocity, prev, stage.gravityReversed);
			if (landed) {
				onGround = true;
				sp.triggered = true;//割れ開始
			}
		}
		else {
			if (ResolveSolidYPlayer(sp.right, player, velocity, prev, stage.gravityReversed)) onGround = true;
			if (ResolveSolidYPlayer(sp.left, player, velocity, prev, stage.gravityReversed)) onGround = true;
		}
	}

	//壊れるブロック
	for (int i = 0; i < stage.breakableBlockCount; i++) {
		if(!stage.breakableBlocks[i].isBroken){
			if(ResolveSolidYPlayer(stage.breakableBlocks[i].rect,player,velocity, prev, stage.gravityReversed)){
				onGround=true;
			}
		}
	}

	//下から触れただけで壊れるブロック（Y方向：上からの着地・横は壊れず当たり判定あり、下から触れたら TryBreakFromBelowOnly 処理済み）
	for (int i = 0; i < stage.bottomBreakBlockCount; i++) {
		auto& bb = stage.bottomBreakBlocks[i];
		if (bb.isBroken) continue;
		if (ResolveSolidYPlayer(bb.rect, player, velocity, prev, stage.gravityReversed)) {
			onGround = true;
		}
	}

	//透明ブロック
	for (int i = 0; i < stage.clearsCount; i++) {
		if (stage.clearBlocks[i].clearflag) {
			if (ResolveSolidYPlayer(stage.clearBlocks[i].rect, player, velocity, prev, stage.gravityReversed)) onGround = true;
		}
	}
	for (int i = 0; i < stage.clearsXCount; i++) {
		if (stage.clearBlocksX[i].clearflag) {
			if (ResolveSolidYPlayer(stage.clearBlocksX[i].rect, player, velocity, prev, stage.gravityReversed)) onGround = true;
		}
	}

	//スイッチによって動作する床
	for (int i = 0; i < stage.switchPlatformCount; i++) {
		auto& sp = stage.switchPlatforms[i];
		if (CheckCollisionRecs(player, sp.SwitchRect)) {
			sp.switchedOn = true;
		}
		// 常に当たり判定を行う（switchedOnは床の動きにのみ影響）
		// Y方向（縦）の当たり判定
		if (ResolveSolidYPlayer(stage.switchPlatforms[i].rect, player, velocity, prev, stage.gravityReversed)){
			onGround = true;
		}
		// X方向（横）の当たり判定
		ResolveSolidX(stage.switchPlatforms[i].rect, player, velocity, prev);
	}

	for (int i = 0; i < stage.tempFloorCount; i++) {
		const auto& tf = stage.tempFloors[i];
		if (!tf.visible) continue;
		if (ResolveSolidYPlayer(tf.rect, player, velocity, prev, stage.gravityReversed)) {// Y方向（縦）の当たり判定
			onGround = true;
		}
		ResolveSolidX(tf.rect, player, velocity, prev);// X方向（横）の当たり判定
	}

	} // ===== if (stage.currentLayer == 0) 終わり =====

	// ===== プレイヤー以外のオブジェクト床判定（レイヤーに関係なく常に実行） =====

	auto LandOnLayeredFloor = [&](float cx, float bottom, float radius, float& outTopY) -> bool {// cx: 中心X座標, bottom: 底面Y座標, radius: 半径, outTopY: 床の上面Y座標を返す
		auto tryFront = [&]() -> bool {
			for (int p = 0; p < stage.platformCount; p++) {
				const auto& plat = stage.platforms[p];
				bool overlapX = (cx + radius > plat.x) && (cx - radius < plat.x + plat.width);
				if (overlapX && bottom > plat.y - 0.1f && bottom < plat.y + 10.0f) {
					outTopY = plat.y;
					return true;
				}
			}
			return false;
		};
		auto tryBack = [&]() -> bool {
			for (int p = 0; p < stage.backPlatformCount; p++) {
				Rectangle bp = ScaleRectCenter(stage.backPlatforms[p], stage.BACK_LAYER_SCALE);
				bool overlapX = (cx + radius > bp.x) && (cx - radius < bp.x + bp.width);
				if (overlapX && bottom > bp.y - 0.1f && bottom < bp.y + 10.0f) {
					outTopY = bp.y;
					return true;
				}
			}
			return false;
		};

		// 現在レイヤー優先
		if (stage.currentLayer == 0) {
			if (tryFront()) return true;
			return tryBack();
		} else {
			if (tryBack()) return true;
			return tryFront();
		}
	};

	//落下文字（文字側の床判定）
	for (int i = 0; i < stage.fallingTextCount; i++) {
		auto& ft = stage.fallingTexts[i];
		if (!ft.isFalling) continue;

		// 手前床
		for (int p = 0; p < stage.platformCount; p++) {
			if (ResolveFallingTextY(stage.platforms[p], ft, prev)) {
				ft.pos.y = ft.rect.y;
			}
		}
		// 奥床
		for (int p = 0; p < stage.backPlatformCount; p++) {
			Rectangle bp = ScaleRectCenter(stage.backPlatforms[p], stage.BACK_LAYER_SCALE);
			if (ResolveFallingTextY(bp, ft, prev)) {
				ft.pos.y = ft.rect.y;
			}
		}
	}

	//アームつき鉄球の床判定
	for (int i = 0; i < stage.moveRotatingBallCount; i++) {
		auto& mrb = stage.moveRotatingBalls[i];
		bool wasOnGround = mrb.onGround;
		mrb.onGround = false;

		Vector2 ballPos = mrb.GetBallPosition();
		float topY = 0.0f;
		if (LandOnLayeredFloor(ballPos.x, ballPos.y + mrb.radius, mrb.radius, topY)) {
			mrb.onGround = true;
			mrb.vel.y = 0.0f;
			mrb.isFalling = false;
			if (!wasOnGround) {
				mrb.reverse = !mrb.reverse;
			}
		}
	}

	//転がる鉄球の床判定 + 壊せるブロック破壊
	for (int i = 0; i < stage.rollingBallCount; i++) {
		auto& rb = stage.rollingBalls[i];

		// ボタン待機中はまだ動かさない（破壊もしない）
		if (rb.waitForCursorClick && !rb.activated) {
			continue;
		}

		const float bottom = rb.center.y + rb.radius;// 鉄球の底面Y座標
		const float prevBottom = bottom - (rb.vel.y * dt);// 前フレームの底面Y座標
		float topY = 0.0f;

		bool landed = LandOnLayeredFloor(rb.center.x, rb.center.y + rb.radius, rb.radius, topY);

		//高速落下対策swept判定
		if (!landed && rb.vel.y > 0.0f) {// 落下中でまだ床に着地していない場合
			auto sweptHit = [&](const Rectangle& plat) -> bool {
				const bool overlapX = (rb.center.x + rb.radius > plat.x) &&
					(rb.center.x - rb.radius < plat.x + plat.width);// X方向の重なり判定
				if (!overlapX) return false;
				
				const float eps = 0.1f;// 許容誤差
				const bool crossedTop = (prevBottom <= plat.y + eps) 
					&& (bottom >= plat.y - eps);// 前フレームの底面が床の上面より上、かつ今フレームの底面が床の上面より下にある場合

				if (crossedTop) {
					topY = plat.y;
					return true;
				}
				return false;
				};
			 
			if (stage.currentLayer == 0) {
				for (int p = 0; p < stage.platformCount && !landed; p++) {
					landed = sweptHit(stage.platforms[p]);
				}  for (int p = 0; p < stage.backPlatformCount && !landed; p++) {
					Rectangle bp = ScaleRectCenter(stage.backPlatforms[p], stage.BACK_LAYER_SCALE);
					landed = sweptHit(bp);
				}
			}
			else {
				for (int p = 0; p < stage.backPlatformCount && !landed; p++) {
					Rectangle bp = ScaleRectCenter(stage.backPlatforms[p], stage.BACK_LAYER_SCALE);
					landed = sweptHit(bp);
				}
				for (int p = 0; p < stage.platformCount && !landed; p++) {
					landed = sweptHit(stage.platforms[p]);
				}
			}
		}


		if (landed) {
			rb.vel.y = 0.0f;
			rb.center.y = topY - rb.radius;
		}
		rb.onGround = landed;

		// 壊せるブロックとの当たり判定（鉄球で破壊）
		for (int b = 0; b < stage.breakableBlockCount; b++) {
			auto& bb = stage.breakableBlocks[b];
			if (bb.isBroken) continue;

			if (CheckCollisionCircleRec(rb.center, rb.radius, bb.rect)) {
				bb.isBroken = true;
			}
		}
		// 落下床との当たり判定（鉄球が触れたら落下開始）
		for (int f = 0; f < stage.fallingCount; f++) {
    auto& fp = stage.fallingPlatforms[f];
    if (fp.triggered || fp.isfalling) continue;

    // detectMode: 0=両方, 1=プレイヤーのみ, 2=ボールのみ
    if (!(fp.detectMode == 0 || fp.detectMode == 2)) continue;

    if (CheckCollisionCircleRec(rb.center, rb.radius, fp.rect)) {
        fp.triggered = true;
        fp.timer = 0.0f;
    }
}
	}

	for (int i = 0; i < stage.craneLaunchPadCount; i++) {
		auto& cl = stage.craneLaunchPads[i];
		bool landed = ResolveSolidYPlayer(cl.rect, player, velocity, prev, stage.gravityReversed);

			if (landed) {
				onGround = true;
				if (!cl.triggered) {
					cl.triggered = true;
					velocity.y = cl.launchVelY;
					onGround = false;
					stage.playerLaunched = true;
				}
			}


			}

			// exitDoor判定
			stage.exitDoorTriggered = -1; // 毎フレーム初期化
			for (int i = 0; i < stage.exitDoorCount; i++) {
				if (CheckCollisionRecs(player, stage.exitDoors[i].rect)) {
					stage.exitDoorTriggered = i;
					break;
				}
			}

			// ワープホール判定
			stage.warpTriggered = -1;  // 毎フレーム初期化
			for (int i = 0; i < stage.warpCount; i++) {
				if (CheckCollisionRecs(player, stage.warps[i].rect) && IsKeyPressed(KEY_W)) {
					stage.warpTriggered = i;  // ワープホールのインデックスを記録
					break;  // 最初にマッチしたワープホールでループを抜ける
				}
			}

			return onGround;
		}

//プレイヤーが氷床に乗っているか判定
bool IsOnIcePlatform(const Stage& stage,const Rectangle player){
	for (int i = 0; i < stage.icePlatformCount; i++) {
		auto& ip = stage.icePlatforms[i];
		float playerBottom = player.y + player.height;
		float iceTop = ip.rect.y;
		float tolerance = 5.0f;//許容値
		bool onTop = (playerBottom >= iceTop - tolerance)&&(playerBottom <= iceTop + tolerance);
		bool overlapX = CheckOverlapX(player, ip.rect);
		if (onTop && overlapX) {
			return true;
		}
	}
	return false;
}
//プレイヤーが移動低下床に乗っているか判定
bool IsOnMoveDownPlatform(const Stage& stage,const Rectangle player){
	for (int i = 0; i < stage.moveDownPlatformCount; i++) {
		auto& md = stage.moveDownPlatforms[i];
		float playerBottom = player.y + player.height;
		float platformTop = md.rect.y;
		float tolerance = 5.0f;//許容値
		bool onTop = (playerBottom >= platformTop - tolerance) && (playerBottom <= platformTop + tolerance);
		bool overlapX = CheckOverlapX(player, md.rect);
		if (onTop && overlapX) {
			return true;
		}
	}
	return false;
}

//プレイヤーが移動上昇床に乗っているか判定 
bool IsOnMoveUpPlatform(const Stage& stage,const Rectangle player){
	for (int i = 0; i < stage.moveUpPlatformCount; i++) {
		auto& mu = stage.moveUpPlatforms[i];
		float playerBottom = player.y + player.height;
		float platformTop = mu.rect.y;
		float tolerance = 5.0f;//許容値
		bool onTop = (playerBottom >= platformTop - tolerance) && (playerBottom <= platformTop + tolerance);
		bool overlapX = CheckOverlapX(player, mu.rect);
		if (onTop && overlapX) {
			return true;
		}
	}
	return false;
}


void MoveUpdateWithPlayrer(Stage& stage, Rectangle& player, Vector2& velocity, float dt) {
	MoveWithPlayer(stage, player, velocity, dt);
	MoveWithUpDownPlatform(stage, player, velocity, dt);
	MoveWithCirclePlatform(stage, player, velocity, dt);
}

void ElevatorUpdate(Stage& stage, Rectangle& player, Vector2& velocity, float dt) {
	stage.playerInElevator = false;

	for (int i = 0; i < stage.elevatorCount; i++) {
		auto& ev = stage.elevators[i];

		Rectangle triggerZone = {
			ev.rect.x,
			ev.rect.y - 20.0f,
			ev.rect.width,
			ev.rect.height + 80.0f
		};
		bool playerOverlap = CheckCollisionRecs(player, triggerZone) || ev.onPlayer;

		if (playerOverlap) {
			stage.playerInElevator = true;

			if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
				if (ev.rect.y <= ev.upperY) continue;
				ev.goingUp = true;
				ev.goingDown = false;
			}
			else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
				if (ev.rect.y >= ev.lowerY) continue;
				ev.goingDown = true;
				ev.goingUp = false;
			}
		}

		if (ev.goingUp) {
			float prevY = ev.rect.y;
			ev.rect.y -= ev.speed * dt;
			if (ev.rect.y <= ev.upperY) {
				ev.rect.y = ev.upperY;
				ev.goingUp = false;
			}
			float deltaY = ev.rect.y - prevY;
			if (playerOverlap && deltaY != 0.0f) {
				player.y += deltaY;
				velocity.y = 0;
			}
		}
		else if (ev.goingDown) {
			float prevY = ev.rect.y;
			ev.rect.y += ev.speed * dt;
			if (ev.rect.y >= ev.lowerY) {
				ev.rect.y = ev.lowerY;
				ev.goingDown = false;
			}
			float deltaY = ev.rect.y - prevY;
			if (playerOverlap && deltaY != 0.0f) {
				player.y += deltaY;
				velocity.y = 0;
			}
		}
	}
}