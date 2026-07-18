#pragma once
#include "StageTypes.h"

// すべての信号をOFFにする
void ClearGimmickSignals(Stage& stage);

// 現在の距離トリガー状態から全信号を再計算する
void RebuildGimmickSignals(Stage& stage);

// 指定された信号がONか
bool IsGimmickSignalActive(const Stage& stage, int signalId);