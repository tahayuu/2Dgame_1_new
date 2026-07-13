#include "StageDraw.h"
#include "Stage.h"
#include "SpriteDatabase.h"
#include <cmath>

// ================================================================
// StageDraw.cpp の役割
// ---------------------------------------------------------------
// ・ステージ内オブジェクトの可視化を一括管理する。
// ・type(挙動)と spriteOverride(見た目)の両方を考慮して描画順を制御する。
// ================================================================

static bool IsAlmostEqual(float a, float b, float eps = 0.01f) {
	return fabsf(a - b) <= eps;
}

static bool IsSameRect(const Rectangle& a, const Rectangle& b) {
	return IsAlmostEqual(a.x, b.x) &&
		IsAlmostEqual(a.y, b.y) &&
		IsAlmostEqual(a.width, b.width) &&
		IsAlmostEqual(a.height, b.height);
}

// spriteInstancesに同じrectのスプライト上書きがあるかどうかを判定する
static bool HasSpriteOverride(const Stage& stage, const Rectangle& rect) {
	for (int i = 0; i < stage.spriteInstanceCount; i++) {// spriteInstancesの末尾ループで二重描画しないよう、動くギミックの初期rectかどうかを判定する
		const auto& si = stage.spriteInstances[i];// spriteIdがNoneなら描画なしなのでスキップ
		if (si.spriteId == SpriteId::None) continue;// rectが一致するなら上書きあり
		if (IsSameRect(si.rect, rect)) return true;// rectが一致しないなら次のspriteInstanceをチェック
	}
	return false;
}

static const Stage::SpriteInstance* FindSpriteOverrideByInitRect(const Stage& stage, const Rectangle& initRect) {
	for (int i = 0; i < stage.spriteInstanceCount; i++) {
		const auto& si = stage.spriteInstances[i];
		if (si.spriteId == SpriteId::None) continue;
		if (IsSameRect(si.rect, initRect)) return &si;
	}
	return nullptr;
}

// 動くギミック用：initRectでスプライトを検索してcurrentRectに描画する
// 見つかればtrueを返す（デフォルト描画をスキップする判定に使う）
static bool TryDrawMovingGimmickSprite(const Stage& stage, const Rectangle& initRect, const Rectangle& currentRect) {
	const auto* si = FindSpriteOverrideByInitRect(stage, initRect);
	if (si == nullptr || si->spriteId == SpriteId::None) return false;
	SpriteDatabase::DrawSprite(si->spriteId, currentRect, si->rotation, si->flipX, si->flipY, WHITE);
	return true;
}

// 動くギミック用針描画（スプライト上書きがあればそちらを、なければデフォルト針を描画）
static void DrawMovingSpikeWithOverride(const Stage& stage, const Rectangle& currentRect, const Rectangle& initRect, float spikeW) {
	if (!TryDrawMovingGimmickSprite(stage, initRect, currentRect)) {
		DrawSpikes(currentRect, spikeW);
	}
}

// spriteInstancesの末尾ループで二重描画しないよう、動くギミックの初期rectかどうかを判定する
static bool IsGimmickDrawnInline(const Stage& stage, const Rectangle& rect) {
	// 動く針系
	for (int i = 0; i < stage.moveCount; i++) if (IsSameRect(stage.moveHazardsInit[i].rect, rect)) return true;
	for (int i = 0; i < stage.moveExtYCount; i++) if (IsSameRect(stage.moveHazardsExtYInit[i].rect, rect)) return true;
	for (int i = 0; i < stage.moveDownHazardExtYCount; i++) if (IsSameRect(stage.moveDownHazardsExtYInit[i].rect, rect)) return true;
	for (int i = 0; i < stage.moveHazardRightCount; i++) if (IsSameRect(stage.moveHazardsRightInit[i].rect, rect)) return true;
	for (int i = 0; i < stage.trackingHazardCount; i++) if (IsSameRect(stage.trackingHazardsInit[i].rect, rect)) return true;
	// エレベーター
	for (int i = 0; i < stage.elevatorCount; i++) if (IsSameRect(stage.elevatorsInit[i].rect, rect)) return true;
	// 落下床
	for (int i = 0; i < stage.fallingCount; i++) if (IsSameRect(stage.fallingPlatformsInit[i].rect, rect)) return true;
	// 上昇床
	for (int i = 0; i < stage.upRisingCount; i++) if (IsSameRect(stage.upRisingPlatformsInit[i].rect, rect)) return true;
	// 往復上昇床
	for (int i = 0; i < stage.upDownCount; i++) if (IsSameRect(stage.upDouwnPlatformsInit[i].rect, rect)) return true;
	// 乗ると上昇する床
	for (int i = 0; i < stage.moveUpPlatformCount; i++) if (IsSameRect(stage.moveUpplatformsInit[i].rect, rect)) return true;
	// 移動低下床
	for (int i = 0; i < stage.moveDownPlatformCount; i++) if (IsSameRect(stage.moveDownplatformsInit[i].rect, rect)) return true;
	// カーソル追従床
	for (int i = 0; i < stage.cursorPlatformCount; i++) if (IsSameRect(stage.cursorplatformsInit[i].rect, rect)) return true;
	// 吹っ飛ばし壁
	for (int i = 0; i < stage.knockBackWallCount; i++) if (IsSameRect(stage.knockBackWallsInit[i].rect, rect)) return true;
	// 乗ると動く床X
	for (int i = 0; i < stage.movePlatformCountX; i++) if (IsSameRect(stage.moveplatformsXInit[i].rect, rect)) return true;
	// スイッチ床
	for (int i = 0; i < stage.switchPlatformCount; i++) if (IsSameRect(stage.switchplatformsInit[i].rect, rect)) return true;
	// わかれる床（base rect）
	for (int i = 0; i < stage.splitPlatformCount; i++) if (IsSameRect(stage.splitplatformsInit[i].base, rect)) return true;
	// クレーン
	for (int i = 0; i < stage.craneCount; i++) if (IsSameRect(stage.cranesInit[i].bodyRect, rect)) return true;
	// 円軌道床（centerベースで判定）
	for (int i = 0; i < stage.circlePlatformCount; i++) {
		float cx = rect.x + rect.width / 2.0f, cy = rect.y + rect.height / 2.0f;
		if (IsAlmostEqual(cx, stage.circleplatformsInit[i].center.x) &&
			IsAlmostEqual(cy, stage.circleplatformsInit[i].center.y)) return true;
	}
	return false;
}

// トゲ描画関数（実装）
void DrawSpikes(Rectangle h, float spikeW) {
	int spikeWCount = (int)(h.width / spikeW);
	if (spikeWCount < 1) spikeWCount = 1;

	for (int s = 0; s < spikeWCount; s++) {
		float x0 = h.x + s * spikeW;

		Vector2 p1 = { x0,               h.y + h.height };
		Vector2 p2 = { x0 + spikeW,      h.y + h.height };
		Vector2 p3 = { x0 + spikeW / 2.0f, h.y };

		DrawTriangle(p1, p2, p3, RED);
	}
}

// 矢印描画関数（テクスチャ版）
static void DrawArrowTexture(const Texture2D& tex, Rectangle rect, float angleDeg) {
	if (tex.id == 0) {
		DrawRectangleRec(rect, GREEN);
		DrawRectangleLinesEx(rect, 2, DARKGREEN);
		return;
	}

	Rectangle src = {
		0.0f, 0.0f,
		(float)tex.width,
		(float)tex.height
	};

	// DrawTexturePro は dst の x,y が「描画位置」ではなく
	// origin と組み合わせるので、中心基準で描く
	Rectangle dst = {
		rect.x + rect.width * 0.5f,
		rect.y + rect.height * 0.5f,
		rect.width,
		rect.height
	};

	Vector2 origin = {
		rect.width * 0.5f,
		rect.height * 0.5f
	};

	DrawTexturePro(tex, src, dst, origin, angleDeg, WHITE);
}

// 目的: ステージ中の全ギミックを適切なレイヤ順で描画する中核関数。
// 入力: stage（現在状態）、spikeW（トゲ幅）、player（一部ギミック描画参照用）。
// 出力: 描画のみ（stageデータは変更しない）。
// 注意: 移動ギミックのスプライト上書きは initRect 基準で対応しているため、
//       Export/Import の初期矩形管理と整合が必要。
void StageDraw(const Stage& stage, float spikeW, const Rectangle& player, int heldSpringIndex) {
	// ===== Tiled タイルレイヤーを最初に描画（背景） =====
	const float bScale = stage.BACK_LAYER_SCALE;



	// ===== 奥レイヤーの床（常に先に描画＝背面） =====
	for (int i = 0; i < stage.backPlatformCount; i++) {
		const auto& p = stage.backPlatforms[i];
		if (HasSpriteOverride(stage, p)) continue;

		// 位置はそのまま、サイズだけ縮小（中心基準）
		float sw = p.width * bScale;
		float sh = p.height * bScale;
		Rectangle drawRect = {
			p.x + (p.width - sw) / 2,
			p.y + (p.height - sh) / 2,
			sw, sh
		};

		if (stage.currentLayer == 1) {
			DrawRectangleRec(drawRect, BEIGE);
			DrawRectangleLinesEx(drawRect, 2, BROWN);
		}
		else {
			DrawRectangleRec(drawRect, ColorAlpha(DARKGRAY, 0.3f));
			DrawRectangleLinesEx(drawRect, 1, ColorAlpha(BLACK, 0.2f));
		}
	}

	// ===== 奥行きドア（常に描画） =====
	for (int i = 0; i < stage.layerDoorCount; i++) {
		const auto& ld = stage.layerDoors[i];

		// 奥のドア（サイズだけ縮小、位置はそのまま）
		{
			const auto& r = ld.backRect;
			float dw = r.width * bScale;
			float dh = r.height * bScale;
			Rectangle dr = {
				r.x + (r.width - dw) / 2,
				r.y + (r.height - dh) / 2,
				dw, dh
			};
			Color c = (stage.currentLayer == 1) ? DARKPURPLE : ColorAlpha(DARKPURPLE, 0.4f);
			DrawRectangleRec(dr, c);
			DrawRectangleLinesEx(dr, 2, PURPLE);
		}
		// 手前のドア（通常サイズ）
		{
			Color c = (stage.currentLayer == 0) ? DARKGREEN : ColorAlpha(DARKGREEN, 0.4f);
			DrawRectangleRec(ld.frontRect, c);
			DrawRectangleLinesEx(ld.frontRect, 3, GREEN);
		}
	}

	// ===== 手前レイヤーの普通床 =====
	for (int i = 0; i < stage.platformCount; i++) {
		if (HasSpriteOverride(stage, stage.platforms[i])) continue;
		if (stage.currentLayer == 0) {
			DrawPlatformTextured(stage.platforms[i], stage.theme);
		}
		else {
			DrawRectangleRec(stage.platforms[i], ColorAlpha(LIGHTGRAY, 0.3f));
			DrawRectangleLinesEx(stage.platforms[i], 1, ColorAlpha(GRAY, 0.25f));
		}
	}

	//氷床
	for (int i = 0; i < stage.icePlatformCount; i++) {
		const auto& ice = stage.icePlatforms[i];
		if (HasSpriteOverride(stage, ice.rect)) continue;
		DrawRectangleRec(stage.icePlatforms[i].rect, SKYBLUE);
		DrawLineEx(
			{ ice.rect.x + 5, ice.rect.y + 3 },
			{ ice.rect.x + ice.rect.width - 5, ice.rect.y + 3 },
			2.0f, WHITE
		);
		// 枠線
		DrawRectangleLinesEx(stage.icePlatforms[i].rect, 2, DARKBLUE);
	}
	//アイテムブロック
	for (int i = 0; i < stage.itemBlockCount; i++) {
		const auto& ib = stage.itemBlocks[i];
		if (HasSpriteOverride(stage, ib.rect)) continue;
		if (stage.theme.itemBlock.id != 0) {
			Rectangle src = { 0, 0, (float)stage.theme.itemBlock.width, (float)stage.theme.itemBlock.height };
			DrawTexturePro(stage.theme.itemBlock, src, ib.rect, { 0, 0 }, 0, WHITE);
		}
		else {
			DrawRectangleRec(ib.rect, YELLOW);
		}
	}

	//カーソルボタン（クリックギミック）
	for (int i = 0; i < stage.cursorBottomCount; i++) {
		const auto& cb = stage.cursorBottoms[i];
		if (HasSpriteOverride(stage, cb.rect)) continue;// spriteOverrideがあれば描画しない
		// ボタンの状態に応じて表示テクスチャを切り替える
		// ・triggered = true（カーソルが当たっている状態でボタンを押した／作動中） → ActionButtn_On
		// ・triggered = false（通常時、またはカーソルが当たっているだけで未クリック）  → ActionButtn_off
		const Texture2D& btnTex = cb.triggered ? stage.theme.actionButtonOn : stage.theme.actionButtonOff;
		if (btnTex.id != 0) {
			Rectangle src = { 0, 0, (float)btnTex.width, (float)btnTex.height };
			DrawTexturePro(btnTex, src, cb.rect, { 0,0 }, 0, WHITE);
		}
		else {
			// ★ テクスチャ未ロード時のフォールバック（画像パス未設定/読込失敗時でも安全に表示する）
			Color col = cb.triggered ? ORANGE : (cb.isActive ? YELLOW : GOLD);
			DrawRectangleRec(cb.rect, col);
			DrawRectangleLinesEx(cb.rect, 2, DARKBROWN);
		}
	}

	//移動低下床
	for (int i = 0; i < stage.moveDownPlatformCount; i++) {
		const auto& md = stage.moveDownPlatforms[i];
		if (!TryDrawMovingGimmickSprite(stage, stage.moveDownplatformsInit[i].rect, md.rect)) {
			DrawRectangleRec(md.rect, BROWN);
			DrawLineEx(
				{ md.rect.x + 5, md.rect.y + 3 },
				{ md.rect.x + md.rect.width - 5,md.rect.y + 3 }, 5.0f, BLACK
			);
		}
	}
	//ジャンプ台
	for (int i = 0; i < stage.jumpPlatfromCount; i++) {
		const auto& jp = stage.jumpPlatfroms[i];
		if (HasSpriteOverride(stage, jp.rect)) continue;
		DrawRectangleRec(jp.rect, GREEN);
	}

	//ふっとばし壁
	for (int i = 0; i < stage.knockBackWallCount; i++) {
		const auto& kbw = stage.knockBackWalls[i];
		if (!TryDrawMovingGimmickSprite(stage, stage.knockBackWallsInit[i].rect, kbw.rect)) {
			DrawRectangleRec(kbw.rect, ORANGE);
		}
	}

	//人を発射する砲台
	for (int i = 0; i < stage.batteryHumanCount; i++) {
		const auto& bh = stage.batteryHumans[i];
		if (HasSpriteOverride(stage, bh.rect)) continue;
		// bulletTex が有効なら Bullet.png テクスチャで描画、なければ従来の色で描画
		if (stage.theme.bulletTex.id != 0) {
			Rectangle src = { 0.0f, 0.0f,
				(float)stage.theme.bulletTex.width,
				(float)stage.theme.bulletTex.height };
			DrawTexturePro(stage.theme.bulletTex, src, bh.rect, { 0.0f, 0.0f }, 0.0f, WHITE);
		}
		else {
			DrawRectangleRec(bh.rect, DARKGREEN);
			DrawLineEx(
				{ bh.rect.x + bh.rect.width, bh.rect.y + bh.rect.height / 2 - 5 },
				{ bh.rect.x + bh.rect.width + 15, bh.rect.y + bh.rect.height / 2 - 5 }, 5.0f, BLACK
			);
		}
	}

	//カーソル追従床
	for (int i = 0; i < stage.cursorPlatformCount; i++) {
		const auto& cp = stage.cursorPlatforms[i];
		if (!TryDrawMovingGimmickSprite(stage, stage.cursorplatformsInit[i].rect, cp.rect)) {
			DrawRectangleRec(cp.rect, GOLD);
		}
	}

	//磁石（引き寄せ）
	for (int i = 0; i < stage.magnetCount; i++) {
		const auto& mg = stage.magnets[i];
		if (HasSpriteOverride(stage, mg.rect)) continue;
		const Texture2D& magTex = mg.triggerd ? stage.theme.magnetEffectTex : stage.theme.magnetTex;
		if(magTex.id != 0) {
			Rectangle src = { 0, 0, (float)magTex.width, (float)magTex.height };
			DrawTexturePro(magTex, src, mg.rect, { 0, 0 }, 0, WHITE);
		}
	}
	//往復上昇床
	for (int i = 0; i < stage.upDownCount; i++) {
		const auto& ud = stage.upDouwnPlatforms[i];
		if (!TryDrawMovingGimmickSprite(stage, stage.upDouwnPlatformsInit[i].rect, ud.rect)) {
			DrawRectangleRec(ud.rect, PURPLE);
			DrawRectangleLinesEx(ud.rect, 2, GREEN);
		}
	}

	//円軌道床
	for (int i = 0; i < stage.circlePlatformCount; i++) {
		const auto& cp = stage.circlePlatforms[i];
		const auto& cpInit = stage.circleplatformsInit[i];
		// centerを基準にした疑似initRectでスプライトを検索
		Rectangle pseudoInit = { cpInit.center.x - cpInit.armLength, cpInit.center.y - cpInit.armLength,
			cpInit.armLength * 2.0f, cpInit.armLength * 2.0f };
		if (!TryDrawMovingGimmickSprite(stage, pseudoInit, cp.GetRect())) {
			DrawRectangleRec(cp.GetRect(), DARKGRAY);
		}
	}
	//乗ると動く床X
	for (int i = 0; i < stage.moveUpPlatformCount; i++) {
		const auto& mu = stage.moveUpPlatforms[i];
		if (!TryDrawMovingGimmickSprite(stage, stage.moveUpplatformsInit[i].rect, mu.rect)) {
			DrawRectangleRec(mu.rect, RED);
			DrawLineEx(
				{ mu.rect.x + 5, mu.rect.y + 3 },
				{ mu.rect.x + mu.rect.width - 5,mu.rect.y + 3 }, 10.0f, ORANGE
			);
		}
	}
	//落下床
	for (int i = 0; i < stage.fallingCount; i++) {
		const auto& fp = stage.fallingPlatforms[i];
		if (!TryDrawMovingGimmickSprite(stage, stage.fallingPlatformsInit[i].rect, fp.rect)) {
			DrawRectangleRec(fp.rect, GRAY);
		}
	}
	//とげ
	for (int i = 0; i < stage.hazardCount; i++) {
		if (HasSpriteOverride(stage, stage.hazards[i])) continue;
		DrawSpikes(stage.hazards[i], spikeW);
	}
	//移動拡張とげY
	for (int i = 0; i < stage.moveExtYCount; i++) {
		const auto& mhY = stage.moveHazardsExtY[i];
		DrawMovingSpikeWithOverride(stage, mhY.rect, stage.moveHazardsExtYInit[i].rect, spikeW);
	}
	//下に下がるとげY
	for (int i = 0; i < stage.moveDownHazardExtYCount; i++) {
		const auto& mdhY = stage.moveDownHazardsExtY[i];
		DrawMovingSpikeWithOverride(stage, mdhY.rect, stage.moveDownHazardsExtYInit[i].rect, spikeW);
	}
	for (int i = 0; i < stage.moveHazardRightCount; i++) {
		const auto& mhr = stage.moveHazardsRight[i];
		DrawMovingSpikeWithOverride(stage, mhr.rect, stage.moveHazardsRightInit[i].rect, spikeW);
	}
	//動くとげ
	for (int i = 0; i < stage.moveCount; i++) {
		const auto& mh = stage.moveHazards[i];
		DrawMovingSpikeWithOverride(stage, mh.rect, stage.moveHazardsInit[i].rect, spikeW);
	}
	//追尾するとげ
	for (int i = 0; i < stage.trackingHazardCount; i++) {
		const auto& th = stage.trackingHazards[i];
		DrawMovingSpikeWithOverride(stage, th.rect, stage.trackingHazardsInit[i].rect, spikeW);
	}
	for (int i = 0; i < stage.spikeBouncerCount; i++) {
		const auto& mhY = stage.spikeBouncers[i];
		if (HasSpriteOverride(stage, mhY.rect)) continue;
		DrawSpikes(mhY.rect, spikeW);
	}
	//わかれる床
	for (int i = 0; i < stage.splitPlatformCount; i++) {
		const auto& sp = stage.splitPlatforms[i];
		Rectangle currentRect = sp.triggered ? sp.left : sp.base;
		if (!TryDrawMovingGimmickSprite(stage, stage.splitplatformsInit[i].base, currentRect)) {
			if (!sp.triggered) {
				DrawRectangleRec(sp.base, BLUE);
			}
			else {
				DrawRectangleRec(sp.left, BLUE);
				DrawRectangleRec(sp.right, BLUE);
			}
		}
	}
	//上昇床
	for (int i = 0; i < stage.upRisingCount; i++) {
		const auto& ur = stage.upRisingPlatforms[i];
		if (!TryDrawMovingGimmickSprite(stage, stage.upRisingPlatformsInit[i].rect, ur.rect)) {
			DrawRectangleRec(ur.rect, DARKGRAY);
		}
	}
	
	// デコレーション矢印
	for (int i = 0; i < stage.decorArrowCount; i++) {
		const auto& a = stage.decorArrows[i];
		DrawArrowTexture(stage.theme.arrowTex, a.rect, a.angleDeg);
	}
	for (int i = 0; i < stage.decoSpriteCount; i++) {
		const auto& ds = stage.decoSprites[i];
		if (ds.spriteId == SpriteId::None) continue;  // テクスチャ未設定はスキップ
		SpriteDatabase::DrawSprite(ds.spriteId, ds.rect, ds.rotation, ds.flipX, ds.flipY, WHITE);
	}

	// ================================================================
	// エディタで設定したスプライト（見た目のみ）を描画
	// 動的ギミックの initRect に紐づく spriteInstance はインライン描画済みなのでスキップ
	// ================================================================
	for (int i = 0; i < stage.spriteInstanceCount; i++) {
		const auto& si = stage.spriteInstances[i];
		if (si.spriteId == SpriteId::None) continue;
		if (IsGimmickDrawnInline(stage, si.rect)) continue;
		// 静的ギミック上書き or 床上書きなどを描画
		SpriteDatabase::DrawSprite(si.spriteId, si.rect, si.rotation, si.flipX, si.flipY, WHITE);
	}

	// 回転する鉄球
	for (int i = 0; i < stage.rotatingBallCount; i++) {
		const auto& rb = stage.rotatingBalls[i];
		Vector2 ballPos = rb.GetBallPosition();
		// 腕（鎖）を描画
		DrawLineEx(rb.center, ballPos, 3.0f, GREEN);

		// 中心点を描画
		DrawCircleV(rb.center, 5.0f, GREEN);

		// 鉄球を描画
		DrawCircleV(ballPos, rb.radius, DARKGRAY);
		DrawCircleV(ballPos, rb.radius - 3.0f, GRAY);
	}


	// 回転して落ちる鉄球
	for (int i = 0; i < stage.moveRotatingBallCount; i++) {
		const auto& mrb = stage.moveRotatingBalls[i];
		Vector2 ballPos = mrb.GetBallPosition();
		// 腕（鎖）を描画
		DrawLineEx(mrb.center, ballPos, 3.0f, GREEN);
		// 中心点を描画
		DrawCircleV(mrb.center, 5.0f, GREEN);
		// 鉄球を描画
		DrawCircleV(ballPos, mrb.radius, DARKGRAY);
		DrawCircleV(ballPos, mrb.radius - 3.0f, GRAY);
		// デバッグ：状態表示
		DrawCircleLines((int)ballPos.x, (int)ballPos.y, mrb.radius + 2, GREEN);
	}

	//床の上を転がる鉄球
	for (int i = 0; i < stage.rollingBallCount; i++) {
		const auto& rb = stage.rollingBalls[i];

		DrawCircleV(rb.center, rb.radius, DARKGRAY);
		DrawCircleV(rb.center, rb.radius - 3.0f, GRAY);
		Vector2 markPos = {
			rb.center.x + (rb.radius - 5.0f) * cosf(rb.angle),
			rb.center.y + (rb.radius - 5.0f) * sinf(rb.angle)
		};
		DrawLineEx(rb.center, markPos, 3.0f, RED);

		if (rb.onGround) {
			DrawCircleLines((int)rb.center.x, (int)rb.center.y, rb.radius + 2, GREEN);
		}
	}
	//落下文字
	for (int i = 0; i < stage.fallingTextCount; i++) {
		const auto& ft = stage.fallingTexts[i];

		// 描画は計測に使った同じフォントで（RecalcRect / Init で font が設定されている前提）
		if (ft.font.texture.id != 0) {
			DrawTextEx(ft.font, ft.text.c_str(), { ft.rect.x, ft.rect.y }, (float)ft.fontSize, 0.0f, RED);
		}
		else {
			// フォント未設定なら安全策で既定フォントを使う
			DrawText(ft.text.c_str(), (int)ft.rect.x, (int)ft.rect.y, ft.fontSize, RED);
		}

		// 当たり判定（デバッグ表示）
		Rectangle hit = ft.GetRect();
		DrawRectangleLinesEx(hit, 2, GREEN);
	}
	//乗ると動く床X
	for (int i = 0; i < stage.movePlatformCountX; i++) {
		const auto& mpx = stage.movePlatformsX[i];
		if (!TryDrawMovingGimmickSprite(stage, stage.moveplatformsXInit[i].rect, mpx.rect)) {
			DrawRectangleRec(mpx.rect, DARKGRAY);
		}
	}
	//スイッチによって動作する床
	for (int i = 0; i < stage.switchPlatformCount; i++) {
		const auto& sw = stage.switchPlatforms[i];
		if (!TryDrawMovingGimmickSprite(stage, stage.switchplatformsInit[i].rect, sw.rect)) {
			DrawRectangleRec(sw.rect, LIGHTGRAY);
		}
	}


	//壊せるブロック
	for (int i = 0; i < stage.breakableBlockCount; i++) {
		const auto& nb = stage.breakableBlocks[i];
		if (nb.isBroken) continue;
		if (HasSpriteOverride(stage, nb.rect)) continue;

		if (stage.theme.normalBlock.id != 0) {
			Rectangle src = { 0, 0, (float)stage.theme.normalBlock.width, (float)stage.theme.normalBlock.height };
			DrawTexturePro(stage.theme.normalBlock, src, nb.rect, { 0, 0 }, 0, WHITE);
		}
		else {
			// ★ テクスチャ未ロード時のフォールバック
			DrawRectangleRec(nb.rect, BROWN);
			DrawRectangleLinesEx(nb.rect, 2.0f, DARKBROWN);
		}
	}


	//透明
	for (int i = 0; i < stage.clearsCount; i++) {
		if (stage.clearBlocks[i].clearflag) {
			Rectangle src = { 0, 0, (float)stage.theme.normalBlock.width, (float)stage.theme.normalBlock.height };
			DrawTexturePro(stage.theme.normalBlock, src, stage.clearBlocks[i].rect, { 0, 0 }, 0, WHITE);
		}
	}

	//透明
	for (int i = 0; i < stage.clearsXCount; i++) {
		if (stage.clearBlocksX[i].clearflag) {
			Rectangle src = { 0, 0, (float)stage.theme.normalBlock.width, (float)stage.theme.normalBlock.height };
			DrawTexturePro(stage.theme.normalBlock, src, stage.clearBlocksX[i].rect, { 0, 0 }, 0, WHITE);
		}

	}
	
	// クリアドア
	for (int i = 0; i < stage.exitDoorCount; i++) {
		const auto& ed = stage.exitDoors[i];
		if (HasSpriteOverride(stage, ed.rect)) continue;
		DrawRectangleRec(ed.rect, BLACK);
		DrawRectangleLinesEx(ed.rect, 3, DARKGRAY);
	}



	// ワープホール
	for (int i = 0; i < stage.warpCount; i++) {
		const auto& warp = stage.warps[i];
		if (HasSpriteOverride(stage, warp.rect)) continue;
		float cx = warp.rect.x + warp.rect.width / 2;
		float cy = warp.rect.y + warp.rect.height / 2;
		float rad = std::min(warp.rect.width, warp.rect.height) / 2;
		DrawCircle((int)cx, (int)cy, (int)(rad * 0.8f), {100, 200, 255, 200});
		DrawCircleLines((int)cx, (int)cy, (int)(rad * 0.6f), {150, 220, 255, 255});
		DrawCircleLines((int)cx, (int)cy, (int)(rad * 0.3f), {150, 220, 255, 255});
	}

	// ボタンブロック
	for (int i = 0; i < stage.buttonBlockCount; i++) {
		const auto& bb = stage.buttonBlocks[i];
		Color btnColor = bb.activated ? GRAY : GOLD;
		DrawRectangleRec(bb.buttonRect, btnColor);
		DrawRectangleLinesEx(bb.buttonRect, 2, DARKGRAY);
		if (bb.activated) {
			for (const auto& br : bb.blockRects) {
				DrawRectangleRec(br, BROWN);
				DrawRectangleLinesEx(br, 2, DARKBROWN);
			}
		}
	}

	//エレベーター
	for (int i = 0; i < stage.elevatorCount; i++) {
		const auto& ev = stage.elevators[i];
		if (!TryDrawMovingGimmickSprite(stage, stage.elevatorsInit[i].rect, ev.rect)) {
			// レール（移動範囲）
			float railX = ev.rect.x + ev.rect.width / 2 - 2;
			DrawRectangle((int)railX, (int)ev.upperY, 4,
				(int)(ev.lowerY - ev.upperY + ev.rect.height), ColorAlpha(GRAY, 0.3f));
			// 本体
			DrawRectangleRec(ev.rect, DARKBLUE);
			DrawRectangleLinesEx(ev.rect, 2, SKYBLUE);
			// 上下矢印
			float cx = ev.rect.x + ev.rect.width / 2;
			float cy = ev.rect.y + ev.rect.height / 2;
			DrawTriangle({ cx - 6, cy }, { cx + 6, cy }, { cx, cy - 8 }, WHITE);
			DrawTriangle({ cx + 6, cy + 4 }, { cx - 6, cy + 4 }, { cx, cy + 12 }, WHITE);
		}
	}

	//重力反転ブロック
	for (int i = 0; i < stage.gravityBlockCount; i++) {
		const auto& gb = stage.gravityBlocks[i];
		if (HasSpriteOverride(stage, gb.rect)) continue;
		DrawRectangleRec(gb.rect, PURPLE);
		//矢印マーク（上下）を描画
		float cx = gb.rect.x + gb.rect.width / 2;
		float cy = gb.rect.y + gb.rect.height / 2;
		DrawTriangle(
			{ cx - 8, cy + 2 }, { cx + 8, cy + 2 }, { cx, cy - 10 }, WHITE);
		DrawTriangle(
			{ cx + 8, cy - 2 }, { cx - 8, cy - 2 }, { cx, cy + 10 }, WHITE);
	}

	//触れると壊れるブロック
	for (int i = 0; i < stage.touchBreakBlockCount; i++) {
		const auto& tb = stage.touchBreakBlocks[i];
		if (tb.isBroken) continue;

		// 崩壊進行度（0.0〜1.0）
		float progress = tb.triggered ? (tb.timer / tb.breakDelay) : 0.0f;
		if (progress > 1.0f) progress = 1.0f;

		// 崩壊中は色が薄くなり、ガタガタ揺れる
		unsigned char alpha = (unsigned char)(255 * (1.0f - progress * 0.6f));
		//Color blockColor = { 180, 120, 60, alpha };// 透明度を下げることで崩壊中の演出'
		//DrawTexturePro(stage.theme.touchBlock, drawRect, stage.touchBreakBlocks[i].rect, { 0, 0 }, 0, WHITE);
		float shakeX = 0.0f;
		if (tb.triggered) {
			shakeX = (float)(GetRandomValue(-2, 2)) * progress;
		}

		Rectangle drawRect = {
			tb.rect.x + shakeX, tb.rect.y,
			tb.rect.width, tb.rect.height
		};
		DrawTexturePro(stage.theme.touchBlock, drawRect, stage.touchBreakBlocks[i].rect, { 0, 0 }, 0, WHITE);

		// ひび割れ線
		if (tb.triggered) {
			float cx = drawRect.x + drawRect.width / 2;
			float cy = drawRect.y + drawRect.height / 2;
			DrawLineEx({ cx - 8, cy - 6 }, { cx + 5, cy + 8 }, 2.0f, DARKBROWN);
			DrawLineEx({ cx + 3, cy - 7 }, { cx - 6, cy + 5 }, 2.0f, DARKBROWN);
		}
	}

	//下から触れただけで壊れるブロック
	for (int i = 0; i < stage.bottomBreakBlockCount; i++) {
		const auto& bb = stage.bottomBreakBlocks[i];
		if (bb.isBroken) continue;
		if (HasSpriteOverride(stage, bb.rect)) continue;

		float progress = bb.triggered ? (bb.timer / bb.breakDelay) : 0.0f;
		if (progress > 1.0f) progress = 1.0f;

		unsigned char alpha = (unsigned char)(255 * (1.0f - progress * 0.6f));
		Color blockColor = { 180, 120, 60, alpha };

		float shakeX = 0.0f;
		if (bb.triggered) {
			shakeX = (float)(GetRandomValue(-2, 2)) * progress;
		}

		Rectangle drawRect = {
			bb.rect.x + shakeX, bb.rect.y,
			bb.rect.width, bb.rect.height
		};
	DrawTexturePro(stage.theme.touchBlock, drawRect, stage.bottomBreakBlocks[i].rect, { 0, 0 }, 0, WHITE);

		if (bb.triggered) {
			float cx = drawRect.x + drawRect.width / 2;
			float cy = drawRect.y + drawRect.height / 2;
			DrawLineEx({ cx - 8, cy - 6 }, { cx + 5, cy + 8 }, 2.0f, DARKBROWN);
			DrawLineEx({ cx + 3, cy - 7 }, { cx - 6, cy + 5 }, 2.0f, DARKBROWN);
		}
	}


	//プレイヤーが持つばね
	for (int i = 0; i < stage.springCount; i++) {
		const auto& sp = stage.springs[i];
		if (!sp.isActive) continue;
		if (HasSpriteOverride(stage, sp.rect)) continue;

		// ばね本体を描画（コイル状）
		DrawRectangleRec(sp.rect, PINK);
		DrawRectangleLinesEx(sp.rect, 2, MAROON);

		// コイルを表現する線を描く
		int coils = 3;
		float coilSpacing = sp.rect.height / (coils + 1);
		for (int c = 1; c <= coils; c++) {
			float y = sp.rect.y + coilSpacing * c;
			DrawLineEx(
				{ sp.rect.x + 2, y },
				{ sp.rect.x + sp.rect.width - 2, y },
				1.5f, MAROON
			);
		}
	}

	//クレーン発射台の描画
	for (int i = 0; i < stage.craneLaunchPadCount; i++) {
		const auto& clp = stage.craneLaunchPads[i];
		if (HasSpriteOverride(stage, clp.rect)) continue;
		DrawRectangleRec(clp.rect, DARKGREEN);
		DrawRectangleLinesEx(clp.rect, 2, GREEN);
	}

	// ===== クレーンの描画 =====
	for (int i = 0; i < stage.craneCount; i++) {
		const auto& cr = stage.cranes[i];

		// 1. クレーン本体（灰色の四角）
		DrawRectangleRec(cr.bodyRect, { 120, 120, 120, 255 });
		DrawRectangleLinesEx(cr.bodyRect, 2, DARKGRAY);

		// 2. アーム（縦線）：本体中心から armLength 分だけ下に伸びる
		float craneCenterX = cr.bodyRect.x + cr.bodyRect.width / 2;
		float armTopY = cr.ceilingY + cr.bodyRect.height; // 本体底面
		float armBottomY = cr.ceilingY + cr.armLength;       // アーム先端

		DrawLineEx(
			{ craneCenterX, armTopY },
			{ craneCenterX, armBottomY },
			3.0f, DARKGRAY
		);

		// 3. フック（アーム先端の小さな四角）：DESCENDING 以降に表示
		if (cr.state != CraneState::IDLE) {
			float hookW = 20, hookH = 16;
			Rectangle hookRect = {
				craneCenterX - hookW / 2,
				armBottomY,
				hookW, hookH
			};
			DrawRectangleRec(hookRect, GRAY);
			DrawRectangleLinesEx(hookRect, 2, BLACK);
		}
	}

	// ===== 一時的に出現する床やスイッチの描画 =====
	for (int i = 0; i < stage.tempFloorCount; i++) {
		const auto& tf = stage.tempFloors[i];
		if (!tf.visible) continue;
		if (HasSpriteOverride(stage, tf.rect)) continue;
		DrawRectangleRec(tf.rect, { 120,220,255,220 });
		DrawRectangleLinesEx(tf.rect, 2, BLUE);
	}
	for (int i = 0; i < stage.tempFloorSwitchCount; i++) {
		const auto& sw = stage.tempFloorSwitches[i];
		if (HasSpriteOverride(stage, sw.rect)) continue;
		const Texture2D& btnTex = sw.triggered ? stage.theme.actionButtonOn : stage.theme.actionButtonOff;
		Rectangle src = { 0, 0, (float)btnTex.width, (float)btnTex.height };
		DrawTexturePro(btnTex, src, sw.rect, { 0,0 }, 0, WHITE);
	}

}
	// ==========UI: 右上にプレイヤーがもつアイテムの表示===========
	void DrawItemUI(const Stage & stage) {
		const float uiX = (float)GetScreenWidth() - 80.0f;
		const float uiY = 20.0f;
		const float uiSize = 60.0f;

		// 枠は常に表示
		DrawRectangle((int)(uiX - uiSize / 2 - 5), (int)(uiY - 5), 70, 70, ColorAlpha(BLACK, 0.6f));
		DrawRectangleLinesEx({ uiX - uiSize / 2 - 5, uiY - 5, 70, 70 }, 2, YELLOW);

		// 所持時のみ中身を描画
		if (stage.heldSpringIndex >= 0 && stage.heldSpringIndex < stage.springCount) {
			Rectangle iconRect = { uiX - uiSize / 2, uiY, uiSize, uiSize };
			DrawRectangleRec(iconRect, PINK);
			DrawRectangleLinesEx(iconRect, 2, MAROON);

			int coils = 2;
			float coilSpacing = uiSize / (coils + 1);
			for (int c = 1; c <= coils; c++) {
				float y = uiY + coilSpacing * c;
				DrawLineEx({ uiX - uiSize / 2 + 2, y }, { uiX + uiSize / 2 - 2, y }, 1.5f, MAROON);
			}

			DrawLineEx({ uiX - 5, uiY }, { uiX + 5, uiY }, 2.0f, RED);
			DrawLineEx({ uiX - 5, uiY + uiSize }, { uiX + 5, uiY + uiSize }, 2.0f, RED);
			DrawText("R", (int)(uiX - 6), (int)(uiY + uiSize + 12), 18, WHITE);
		}
		else {
			DrawText("-", (int)(uiX - 4), (int)(uiY + 20), 24, GRAY);
		}
	}


