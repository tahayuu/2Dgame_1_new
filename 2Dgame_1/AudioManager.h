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

    Sound jumpSe{};
    Sound damageSe{};
    Sound stageChooseSe{};
    Sound stageDecideSe{};
	Sound tabclickSe{};

    float titleMaxVol = 0.30f;
    float stageMaxVol = 0.35f;
    float fadeSpeed = 1.2f;

    float titleVol = 0.30f;
    float stageVol = 0.0f;

    bool initialized = false;
};

bool AudioInit(AudioManager& a);
void AudioUpdate(AudioManager& a, bool playTitleBgm, bool playStageBgm, float dt);
void AudioPlaySfx(AudioManager& a, SfxId id);
void AudioShutdown(AudioManager& a);