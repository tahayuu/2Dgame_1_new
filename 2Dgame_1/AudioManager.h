#pragma once
#include "raylib.h"

// ================================================================
// AudioManager.h の役割
// ---------------------------------------------------------------
// ・BGM/SEリソースと音量フェード状態をまとめて管理する。
// ・main からは初期化/更新/再生/終了APIのみを呼ぶ想定。
// ================================================================

enum class SfxId {
    Jump,
    Damage,
    StageChoose,
    StageDecide,
    tabclick,
    Punch,
};

struct AudioManager {
	Music titleBgm{};
	Music stageBgm{};
	Music stage3Bgm{};
	Music chooseStageBgm{};
	Music stage4Bgm{};

	Sound jumpSe{};
	Sound damageSe{};
	Sound stageChooseSe{};
	Sound stageDecideSe{};
	Sound tabclickSe{};
	Sound punch{};

	float titleMaxVol = 0.30f;
	float stageMaxVol = 0.35f;
	float stage3MaxVol = 0.35f;
	float chooseStageMaxVol = 0.35f;
	float stage4MaxVol = 0.35f;
	float editorBgmVol = 0.12f;  // エディタ/UI時の低い音量
	float fadeSpeed = 1.2f;

    float titleVol = 0.30f;
    float stageVol = 0.0f;
    float stage3Vol = 0.0f;
    float chooseStageVol = 0.0f;
    float stage4Vol = 0.0f;

    bool initialized = false;
};

// 目的: オーディオデバイスとBGM/SEを初期化する。
bool AudioInit(AudioManager& a);
// 目的: 各BGMの目標音量へフェードさせながら再生状態を更新する。
void AudioUpdate(AudioManager& a, bool playTitleBgm, bool playStageBgm, bool playStage3Bgm, bool playStage4Bgm,
    bool playChooseStageBgm, bool inEditor, bool inUITab, float dt);
// 目的: 指定SEを再生する。
void AudioPlaySfx(AudioManager& a, SfxId id);
// 目的: すべての音声リソースを解放する。
void AudioShutdown(AudioManager& a);
