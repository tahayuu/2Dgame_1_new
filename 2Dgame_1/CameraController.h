#pragma once
#include "raylib.h"

// ステージごとのカメラ挙動設定
struct CameraConfig {
    bool  twoLayered = false;  // 2段構造ステージか（true=地上/地下で固定、シャフトで追従）
    float shaftEnterY = 0.0f;  // シャフト開始Y (0=stageHeightから自動計算)
    float shaftExitY = 0.0f;  // シャフト終了Y (0=stageHeightから自動計算)
    float lerpSpeed = 10.0f; // ゾーン切替時の補間速度
};

// カメラ更新（main.cpp から毎フレーム呼ぶ）
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