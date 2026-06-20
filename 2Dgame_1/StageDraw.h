#pragma once
#include "raylib.h"

// 前方宣言
struct Stage;

// トゲ描画関数
void DrawSpikes(Rectangle h, float spikeW);

// ステージ全体の描画（ワールド座標）
void StageDraw(const Stage& stage, float spikeW, const Rectangle& player, int heldSpringIndex);

// アイテムUI描画（スクリーン座標）
void DrawItemUI(const Stage& stage);
