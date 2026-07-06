#pragma once
#include "raylib.h"

enum class SfxId {
    Jump,
    Damage,
    StageChoose,
    StageDecide,
    tabclick,
};

struct AudioManager {
	Music titleBgm{};
	Music stageBgm{};
	Music stage3Bgm{};
	Music chooseStageBgm{};

	Sound jumpSe{};
	Sound damageSe{};
	Sound stageChooseSe{};
	Sound stageDecideSe{};
	Sound tabclickSe{};


	float titleMaxVol = 0.30f;
	float stageMaxVol = 0.35f;
	float stage3MaxVol = 0.35f;
	float chooseStageMaxVol = 0.35f;
	float editorBgmVol = 0.12f;  // エディタ/UI時の低い音量
	float fadeSpeed = 1.2f;

    float titleVol = 0.30f;
    float stageVol = 0.0f;
    float stage3Vol = 0.0f;
    float chooseStageVol = 0.0f;

    bool initialized = false;
};

bool AudioInit(AudioManager& a);
void AudioUpdate(AudioManager& a, bool playTitleBgm, bool playStageBgm, bool playStage3Bgm,
    bool playChooseStageBgm, bool inEditor, bool inUITab, float dt);
void AudioPlaySfx(AudioManager& a, SfxId id);
void AudioShutdown(AudioManager& a);