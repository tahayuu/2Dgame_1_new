#include "AudioManager.h"

static float Approach(float cur, float target, float delta) {
    if (cur < target) {
        cur += delta;
        if (cur > target) cur = target;
    }
    else if (cur > target) {
        cur -= delta;
        if (cur < target) cur = target;
    }
    return cur;
}

bool AudioInit(AudioManager& a) {
    InitAudioDevice();

    a.titleBgm = LoadMusicStream("assets/BGM/titleBgm.mp3");
    a.stageBgm = LoadMusicStream("assets/BGM/stage1.mp3");

    a.jumpSe = LoadSound("assets/SE/jump.wav");
    a.damageSe = LoadSound("assets/SE/damage.wav");
    a.stageChooseSe = LoadSound("assets/SE/stageChoose.wav");
    a.stageDecideSe = LoadSound("assets/SE/stageDecide.wav");
	a.tabclickSe = LoadSound("assets/SE/tabclick.wav");

    a.titleBgm.looping = true;
    a.stageBgm.looping = true;

    a.titleVol = a.titleMaxVol;
    a.stageVol = 0.0f;

    SetMusicVolume(a.titleBgm, a.titleVol);
    SetMusicVolume(a.stageBgm, a.stageVol);

    PlayMusicStream(a.titleBgm);
    a.initialized = true;
    return true;
}

void AudioUpdate(AudioManager& a, bool playTitleBgm, bool playStageBgm, float dt) {
    if (!a.initialized) return;

    const float targetTitle = playTitleBgm ? a.titleMaxVol : 0.0f;
    const float targetStage = playStageBgm ? a.stageMaxVol : 0.0f;

    if ((targetTitle > 0.0f || a.titleVol > 0.001f) && !IsMusicStreamPlaying(a.titleBgm)) PlayMusicStream(a.titleBgm);
    if ((targetStage > 0.0f || a.stageVol > 0.001f) && !IsMusicStreamPlaying(a.stageBgm)) PlayMusicStream(a.stageBgm);

    const float step = a.fadeSpeed * dt;
    a.titleVol = Approach(a.titleVol, targetTitle, step);
    a.stageVol = Approach(a.stageVol, targetStage, step);

    SetMusicVolume(a.titleBgm, a.titleVol);
    SetMusicVolume(a.stageBgm, a.stageVol);

    UpdateMusicStream(a.titleBgm);
    UpdateMusicStream(a.stageBgm);

    if (a.titleVol <= 0.001f && targetTitle <= 0.0f && IsMusicStreamPlaying(a.titleBgm)) StopMusicStream(a.titleBgm);
    if (a.stageVol <= 0.001f && targetStage <= 0.0f && IsMusicStreamPlaying(a.stageBgm)) StopMusicStream(a.stageBgm);
}

void AudioPlaySfx(AudioManager& a, SfxId id) {
    if (!a.initialized) return;

    switch (id) {
    case SfxId::Jump:   PlaySound(a.jumpSe); break;
    case SfxId::Damage: PlaySound(a.damageSe); break;
    case SfxId::StageChoose: PlaySound(a.stageChooseSe); break;
    case SfxId::StageDecide: PlaySound(a.stageDecideSe); break;
	case SfxId::tabclick: PlaySound(a.tabclickSe); break;

    default: break;
    }
}

void AudioShutdown(AudioManager& a) {
    if (!a.initialized) return;

    StopMusicStream(a.titleBgm);
    StopMusicStream(a.stageBgm);

    UnloadMusicStream(a.titleBgm);
    UnloadMusicStream(a.stageBgm);

    UnloadSound(a.jumpSe);
    UnloadSound(a.damageSe);
    UnloadSound(a.stageChooseSe);
    UnloadSound(a.stageDecideSe);
	UnloadSound(a.tabclickSe);


    CloseAudioDevice();
    a = AudioManager{};
}