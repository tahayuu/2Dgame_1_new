#pragma once
#include "raylib.h"
#include "GameState.h"
#include "AudioManager.h"

// ================================================================
// TitleScene.h の役割
// ---------------------------------------------------------------
// ・タイトル画面1 / タイトル画面2の状態を保持する。
// ・タイトル画面1ではビルド設定に応じて選択項目を切り替える。
// ・タイトル画面2では「ステージせんたく」だけを選べるようにする。
// ・ステージ選択結果は main 側で読み取り、ロード処理へ渡す。
// ================================================================

// ゲームの物語進行によって切り替わるタイトル画面
// EASY_GAME : 最初の「やさしいゲーム」タイトル
// NO_MORE_EASY : 会話イベント後の「イージーなんていわせない」タイトル
enum class TitleVersion {
    EASY_GAME,
    NO_MORE_EASY
};

// タイトル画面が持つデータをまとめる
struct TitleScene {
    Texture2D title1Bg{};    // タイトル画面1「やさしいゲーム」
    Texture2D title2Bg{};    // タイトル画面2「イージーなんていわせない」
    Font* font = nullptr;    // 借り物フォント（main が持つ jpFont を指す）
    AudioManager* audio = nullptr; // 借り物（main が持つ audio を指す）

    TitleVersion version = TitleVersion::EASY_GAME;
    bool allowAllStagesForDebug = false;

    int selectStage = 0;
    bool isSelectingStage = false;
    static constexpr int STAGE_COUNT = 6;
    static constexpr int STAGE_SELECT_INDEX = 4; // 「ステージせんたく」の実データ番号
};

// 初期化：2枚のタイトル背景、フォント、AudioManager、デバッグ設定を渡す
void TitleSceneInit(TitleScene& ts, Texture2D title1Bg, Texture2D title2Bg, Font& font, AudioManager& audio, bool allowAllStagesForDebug);

// 現在のタイトル状態で画面に表示する選択項目数を返す
int TitleSceneGetSelectableStageCount(const TitleScene& ts);

// タイトル画面1 / 2を切り替える
void TitleSceneSetVersion(TitleScene& ts, TitleVersion version);

// 更新：入力処理。ゲーム開始したら gameState を書き換える
// 戻り値 true = ステージ開始が選ばれた（LoadSelectedStage を呼ぶタイミング）
bool TitleSceneUpdate(TitleScene& ts, GameState& gameState, float dt);

// 描画
void TitleSceneDraw(const TitleScene& ts, int screenWidth, int screenHeight);

// 後処理
void TitleSceneUnload(TitleScene& ts);