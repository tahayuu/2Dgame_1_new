#pragma once
#include "StageTypes.h"

bool UpdateSnapPuzzles(Stage& stage, Camera2D camera, float dt);// true=スナップパズルの状態が変化した
void DrawSnapPuzzles(const Stage& stage);
void InitializeSnapPuzzles(Stage& stage);
void ResetSnapPuzzles(Stage& stage);
bool IsSnapGroupSolved(const Stage& stage, int groupId);
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