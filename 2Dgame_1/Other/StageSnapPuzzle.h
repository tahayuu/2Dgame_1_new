#pragma once
#include "StageTypes.h"

// 通常のスナップパズル
bool UpdateSnapPuzzles(Stage& stage, Camera2D camera, float dt);
void DrawSnapPuzzles(const Stage& stage);
void InitializeSnapPuzzles(Stage& stage);
void ResetSnapPuzzles(Stage& stage);

// 距離トリガーピース
bool UpdateDistanceTriggerPieces(
    Stage& stage,
    Camera2D camera,
    float dt,
    bool allowMouseInput
);

void DrawDistanceTriggerPieces(const Stage& stage);
void InitializeDistanceTriggerPieces(Stage& stage);
void ResetDistanceTriggerPieces(Stage& stage);

// 判定
bool IsSnapGroupSolved(const Stage& stage, int groupId);
bool IsHazardDisabledBySnapPuzzle(const Stage& stage, int hazardIndex);
bool IsHazardDisabledBySnapPuzzle(const Stage& stage, int hazardIndex);
/*部品をクリック
        ↓
マウスへ追従
        ↓
別スロットのsnapRadius内に入る
        ↓
自動的にドラッグを終了
        ↓
0.15秒かけて中央へ吸着
        ↓
部品が入っていれば交換
        ↓
正解判定
        ↓
正解後に部品を固定


*/