#pragma once
#include "raylib.h"

// ================================================================
// CameraController.h の役割
// ---------------------------------------------------------------
// ・プレイヤー位置とステージ寸法に基づくカメラ追従設定を定義する。
// ・2層ステージ（地上/地下）向けのゾーン固定挙動も扱う。
// ================================================================

// ステージごとのカメラ挙動設定
struct CameraConfig {
    bool  twoLayered = false;  // 2段構造ステージか（true=地上/地下で固定、シャフトで追従）
    float shaftEnterY = 0.0f;  // シャフト開始Y (0=stageHeightから自動計算)
    float shaftExitY = 0.0f;  // シャフト終了Y (0=stageHeightから自動計算)
    float lerpSpeed = 10.0f; // ゾーン切替時の補間速度
	bool  extendUpperFollow = false;// true=上方向にプレイヤーが出ても追従する、false=上方向は固定
};

// 目的: カメラtargetを更新し、横方向クランプと縦方向ゾーン補間を適用する。
// 注意: twoLayered 有効時は shaftEnterY/shaftExitY の設定が体感に直結する。
void UpdateCamera(
    Camera2D& camera,
    const Rectangle& player,
    const CameraConfig& config,
    float stageWidth,
    float stageHeight,
    int   screenWidth,
    int   screenHeight,
    float dt
);