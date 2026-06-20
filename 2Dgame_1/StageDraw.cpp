#include "StageDraw.h"
#include "Stage.h"
#include <cmath>

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




// 描画
void StageDraw(const Stage& stage, float spikeW, const Rectangle& player, int heldSpringIndex) {
	// ===== Tiled タイルレイヤーを最初に描画（背景） =====
	const float bScale = stage.BACK_LAYER_SCALE;



	// ===== 奥レイヤーの床（常に先に描画＝背面） =====
	for (int i = 0; i < stage.backPlatformCount; i++) {
		const auto& p = stage.backPlatforms[i];

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
		Color col = cb.triggered ? ORANGE : (cb.isActive ? YELLOW : GOLD);
		DrawRectangleRec(cb.rect, col);
		DrawRectangleLinesEx(cb.rect, 2, DARKBROWN);
	}

	//移動低下床
	for (int i = 0; i < stage.moveDownPlatformCount; i++) {
		const auto& md = stage.moveDownPlatforms[i];
		DrawRectangleRec(md.rect, BROWN);
		DrawLineEx(
			{ md.rect.x + 5, md.rect.y + 3 },
			{ md.rect.x + md.rect.width - 5,md.rect.y + 3 }, 5.0f, BLACK
		);
	}
	//ジャンプ台
	for (int i = 0; i < stage.jumpPlatfromCount; i++) {
		const auto& jp = stage.jumpPlatfroms[i];
		DrawRectangleRec(jp.rect, GREEN);
	}

	//ふっとばし壁
	for (int i = 0; i < stage.knockBackWallCount; i++) {
		const auto& kbw = stage.knockBackWalls[i];
		DrawRectangleRec(kbw.rect, ORANGE);
	}

	//人を発射する砲台
	for (int i = 0; i < stage.batteryHumanCount; i++) {
		const auto& bh = stage.batteryHumans[i];
		DrawRectangleRec(bh.rect, DARKGREEN);
		DrawLineEx(
			{ bh.rect.x + bh.rect.width, bh.rect.y + bh.rect.height / 2 - 5 },
			{ bh.rect.x + bh.rect.width + 15, bh.rect.y + bh.rect.height / 2 - 5 }, 5.0f, BLACK
		);
	}

	//カーソル追従床
	for (int i = 0; i < stage.cursorPlatformCount; i++) {
		const auto& cp = stage.cursorPlatforms[i];
		DrawRectangleRec(cp.rect, GOLD);
	}

	//磁石（引き寄せ）
	for (int i = 0; i < stage.magnetCount; i++) {
		const auto& mg = stage.magnets[i];
		DrawRectangleRec(mg.rect, DARKPURPLE);
		DrawLineEx(
			{ mg.rect.x + 5, mg.rect.y + mg.rect.height / 2 },
			{ mg.rect.x + mg.rect.width - 5,mg.rect.y + mg.rect.height / 2 }, 5.0f, RED
		);
	}
	//往復上昇床
	for (int i = 0; i < stage.upDownCount; i++) {
		const auto& ud = stage.upDouwnPlatforms[i];
		DrawRectangleRec(ud.rect, PURPLE);
		Rectangle hit = ud.rect;
		DrawRectangleLinesEx(hit, 2, GREEN);
	}

	//円軌道床
	for (int i = 0; i < stage.circlePlatformCount; i++) {
		const auto& cp = stage.circlePlatforms[i];
		DrawRectangleRec(cp.GetRect(), DARKGRAY);
	}
	//乗ると動く床X
	for (int i = 0; i < stage.moveUpPlatformCount; i++) {
		const auto& mu = stage.moveUpPlatforms[i];
		DrawRectangleRec(mu.rect, RED);
		DrawLineEx(
			{ mu.rect.x + 5, mu.rect.y + 3 },
			{ mu.rect.x + mu.rect.width - 5,mu.rect.y + 3 }, 10.0f, ORANGE
		);
	}
	//落下床
	for (int i = 0; i < stage.fallingCount; i++) {
		DrawRectangleRec(stage.fallingPlatforms[i].rect, GRAY);
	}
	//とげ
	for (int i = 0; i < stage.hazardCount; i++) {
		DrawSpikes(stage.hazards[i], spikeW);
	}
	//移動拡張とげY
	for (int i = 0; i < stage.moveExtYCount; i++) {
		const auto& mhY = stage.moveHazardsExtY[i];
		DrawSpikes(mhY.rect, spikeW);
	}
	//下に下がるとげY
	for (int i = 0; i < stage.moveDownHazardExtYCount; i++) {
		const auto& mdhY = stage.moveDownHazardsExtY[i];
		DrawSpikes(mdhY.rect, spikeW);
	}
	//移動拡張とげX
	for (int i = 0; i < stage.moveExtXCount; i++) {
		const auto& mhX = stage.moveHazardsExtX[i];
		DrawSpikes(mhX.rect, spikeW);
	}
	for (int i = 0; i < stage.moveHazardRightCount; i++) {
		const auto& mhr = stage.moveHazardsRight[i];
		DrawSpikes(mhr.rect, spikeW);
	}
	//動くとげ
	for (int i = 0; i < stage.moveCount; i++) {
		const auto& mh = stage.moveHazards[i];
		DrawSpikes(mh.rect, spikeW);
	}
	//追尾するとげ
	for (int i = 0; i < stage.trackingHazardCount; i++) {
		const auto& th = stage.trackingHazards[i];
		DrawSpikes(th.rect, spikeW);
	}
	for (int i = 0; i < stage.spikeBouncer​Count; i++) {
		const auto& mhY = stage.spikeBouncers[i];
		DrawSpikes(mhY.rect, spikeW);
	}
	//わかれる床
	for (int i = 0; i < stage.splitPlatformCount; i++) {
		const auto& sp = stage.splitPlatforms[i];
		if (!sp.triggered) {
			DrawRectangleRec(sp.base, BLUE);
		}
		else {
			DrawRectangleRec(sp.left, BLUE);
			DrawRectangleRec(sp.right, BLUE);
		}
	}
	//上昇床
	for (int i = 0; i < stage.upRisingCount; i++) {
		const auto& ur = stage.upRisingPlatforms[i];
		DrawRectangleRec(ur.rect, DARKGRAY);
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
		DrawRectangleRec(mpx.rect, DARKGRAY);
	}
	//スイッチによって動作する床
	for (int i = 0; i < stage.switchPlatformCount; i++) {
		if (stage.switchPlatforms[i].switchedOn) {
			DrawRectangleRec(stage.switchPlatforms[i].rect, LIGHTGRAY);
		}
		else {
			DrawRectangleRec(stage.switchPlatforms[i].rect, LIGHTGRAY);
		}
		//DrawRectangleRec(stage.switchPlatforms[i].SwitchRect, GREEN);
	}
	//壊せるブロック
	for (int i = 0; i < stage.breakableBlockCount; i++) {
		const auto& nb = stage.breakableBlocks[i];
		if (nb.isBroken) continue;

		if (stage.theme.nomalBlock.id != 0) {
			Rectangle src = { 0, 0, (float)stage.theme.nomalBlock.width, (float)stage.theme.nomalBlock.height };
			DrawTexturePro(stage.theme.nomalBlock, src, nb.rect, { 0, 0 }, 0, WHITE);
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
			DrawRectangleRec(stage.clearBlocks[i].rect, GRAY);
		}
		//else DrawRectangleRec(stage.clearBlocks[i].rect, BLUE);
	}

	//透明
	for (int i = 0; i < stage.clearsXCount; i++) {
		if (stage.clearBlocksX[i].clearflag) {
			DrawRectangleRec(stage.clearBlocksX[i].rect, GRAY);
		}

	}

	for (int i = 0; i < stage.exitDoorCount; i++) {
		const auto& ed = stage.exitDoors[i];
		DrawRectangleRec(ed.rect, BLACK);
		DrawRectangleLinesEx(ed.rect, 3, DARKGRAY);
	}

	// ワープホール
	for (int i = 0; i < stage.warpCount; i++) {
		const auto& warp = stage.warps[i];
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

	//重力反転ブロック
	for (int i = 0; i < stage.gravityBlockCount; i++) {
		const auto& gb = stage.gravityBlocks[i];
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
		Color blockColor = { 180, 120, 60, alpha };

		float shakeX = 0.0f;
		if (tb.triggered) {
			shakeX = (float)(GetRandomValue(-2, 2)) * progress;
		}

		Rectangle drawRect = {
			tb.rect.x + shakeX, tb.rect.y,
			tb.rect.width, tb.rect.height
		};
		DrawRectangleRec(drawRect, blockColor);
		DrawRectangleLinesEx(drawRect, 2, BROWN);

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
		DrawRectangleRec(drawRect, blockColor);
		DrawRectangleLinesEx(drawRect, 2, BROWN);

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


