#pragma once
#include "raylib.h"
#include "GameState.h"
#include "AudioManager.h"

// タイトル画面が持つデータをまとめる
struct TitleScene {
    Texture2D bg{};          // タイトル背景画像
    Font* font = nullptr;    // 借り物フォント（main が持つ jpFont を指す）
    AudioManager* audio = nullptr; // 借り物（main が持つ audio を指す）

    int  selectStage = 0;
    bool isSelectingStage = false;
    const int stageCount = 6;
};

// 初期化：背景テクスチャとフォントを渡す
void TitleSceneInit(TitleScene& ts, Texture2D bg, Font& font, AudioManager& audio);

// 更新：入力処理。ゲーム開始したら gameState を書き換える
// 戻り値 true = ステージ開始が選ばれた（LoadSelectedStage を呼ぶタイミング）
bool TitleSceneUpdate(TitleScene& ts, GameState& gameState, float dt);

// 描画
void TitleSceneDraw(const TitleScene& ts, int screenWidth, int screenHeight);

// 後処理
void TitleSceneUnload(TitleScene& ts);