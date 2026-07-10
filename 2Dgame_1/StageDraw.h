#pragma once
#include "raylib.h"

// 前方宣言
struct Stage;

// ================================================================
// StageDraw.h の役割
// ---------------------------------------------------------------
// ・ステージ全体の描画APIを公開する。
// ・当たり判定は扱わず、見た目の描き分けだけを担当する。
// ================================================================

// トゲ描画関数
// 目的: トゲ矩形を三角形で描画する共通ヘルパー。
void DrawSpikes(Rectangle h, float spikeW);

// ステージ全体の描画（ワールド座標）
// 目的: ステージ全体をワールド座標で描画する。
// 注意: sprite上書き描画と通常描画の順序を崩すと二重描画になりやすい。
void StageDraw(const Stage& stage, float spikeW, const Rectangle& player, int heldSpringIndex);

// アイテムUI描画（スクリーン座標）
// 目的: アイテムUIをスクリーン座標で描画する。
void DrawItemUI(const Stage& stage);
