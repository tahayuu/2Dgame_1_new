#include "AudioManager.h"

// ================================================================
// AudioManager.cpp の役割
// ---------------------------------------------------------------
// ・BGMフェード制御とSE再生を実装する。
// ・シーン遷移時に音を切らずに移すため、毎フレームの目標音量追従で制御する。
// ================================================================

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
	a.stage3Bgm = LoadMusicStream("assets/BGM/stage3.mp3");
	a.chooseStageBgm = LoadMusicStream("assets/BGM/chooseStage.mp3");
    a.stage4Bgm = LoadMusicStream("assets/BGM/stage4.mp3");

    a.jumpSe = LoadSound("assets/SE/jump.wav");
    a.damageSe = LoadSound("assets/SE/damage.wav");
    a.stageChooseSe = LoadSound("assets/SE/stageChoose.wav");
    a.stageDecideSe = LoadSound("assets/SE/stageDecide.wav");
	a.tabclickSe = LoadSound("assets/SE/tabclick.wav");
	a.punch = LoadSound("assets/SE/punch_1.wav");
 
    a.titleBgm.looping = true;
    a.stageBgm.looping = true;
	a.stage3Bgm.looping = true;
	a.chooseStageBgm.looping = true;
    a.stage4Bgm.looping = true;

    a.titleVol = a.titleMaxVol;
    a.stageVol = 0.0f;

    SetMusicVolume(a.titleBgm, a.titleVol);
    SetMusicVolume(a.stageBgm, a.stageVol);
    SetMusicVolume(a.stage3Bgm, a.stageVol);
    SetMusicVolume(a.chooseStageBgm, a.stageVol);
    SetMusicVolume(a.stage4Bgm, a.stageVol);

    PlayMusicStream(a.titleBgm);
    a.initialized = true;
    return true;
}
void AudioUpdate(AudioManager& a, bool playTitleBgm, bool playStageBgm, bool playStage3Bgm,bool playStage4Bgm, bool playChooseStageBgm, bool inEditor, bool inUITab, float dt) {
	if (!a.initialized) return;

	// エディタ中またはUI タブ中は音量を下げる
	float volumeScale = (inEditor || inUITab) ? a.editorBgmVol : 1.0f;

	const float targetTitle = playTitleBgm ? (a.titleMaxVol * volumeScale) : 0.0f;
	const float targetStage = playStageBgm ? (a.stageMaxVol * volumeScale) : 0.0f;
	const float targetStage3 = playStage3Bgm ? (a.stage3MaxVol * volumeScale) : 0.0f;
	const float targetStage4 = playStage4Bgm ? (a.stage4MaxVol * volumeScale) : 0.0f;
	const float targetChooseStage = playChooseStageBgm ? (a.chooseStageMaxVol * volumeScale) : 0.0f;

    if ((targetTitle > 0.0f || a.titleVol > 0.001f) && !IsMusicStreamPlaying(a.titleBgm)) PlayMusicStream(a.titleBgm);
    if ((targetStage > 0.0f || a.stageVol > 0.001f) && !IsMusicStreamPlaying(a.stageBgm)) PlayMusicStream(a.stageBgm);
    if ((targetStage3 > 0.0f || a.stage3Vol > 0.001f) && !IsMusicStreamPlaying(a.stage3Bgm)) PlayMusicStream(a.stage3Bgm);
    if ((targetStage4 > 0.0f || a.stage4Vol > 0.001f) && !IsMusicStreamPlaying(a.stage4Bgm)) PlayMusicStream(a.stage4Bgm);
    if ((targetChooseStage > 0.0f || a.chooseStageVol > 0.001f) && !IsMusicStreamPlaying(a.chooseStageBgm)) PlayMusicStream(a.chooseStageBgm);

    const float step = a.fadeSpeed * dt;
    a.titleVol = Approach(a.titleVol, targetTitle, step);
    a.stageVol = Approach(a.stageVol, targetStage, step);
    a.stage3Vol = Approach(a.stage3Vol, targetStage3, step);
    a.chooseStageVol = Approach(a.chooseStageVol, targetChooseStage, step);
    a.stage4Vol = Approach(a.stage4Vol, targetStage4, step);

    SetMusicVolume(a.titleBgm, a.titleVol);
    SetMusicVolume(a.stageBgm, a.stageVol);
    SetMusicVolume(a.stage3Bgm, a.stage3Vol);
	SetMusicVolume(a.chooseStageBgm, a.chooseStageVol);
    SetMusicVolume(a.stage4Bgm, a.stage4Vol);

    UpdateMusicStream(a.titleBgm);
    UpdateMusicStream(a.stageBgm);
	UpdateMusicStream(a.stage3Bgm);
	UpdateMusicStream(a.chooseStageBgm);
	UpdateMusicStream(a.stage4Bgm);

    if (a.titleVol <= 0.001f && targetTitle <= 0.0f && IsMusicStreamPlaying(a.titleBgm)) StopMusicStream(a.titleBgm);
    if (a.stageVol <= 0.001f && targetStage <= 0.0f && IsMusicStreamPlaying(a.stageBgm)) StopMusicStream(a.stageBgm);
    if (a.stage3Vol <= 0.001f && targetStage3 <= 0.0f && IsMusicStreamPlaying(a.stage3Bgm))
        StopMusicStream(a.stage3Bgm);
	if (a.stage4Vol <= 0.001f && targetStage4 <= 0.0f && IsMusicStreamPlaying(a.stage4Bgm))
		StopMusicStream(a.stage4Bgm);
    if (a.chooseStageVol <= 0.001f && targetChooseStage <= 0.0f && IsMusicStreamPlaying(a.chooseStageBgm))
        StopMusicStream(a.chooseStageBgm);
}

void AudioPlaySfx(AudioManager& a, SfxId id) {
    if (!a.initialized) return;

    switch (id) {
    case SfxId::Jump:   PlaySound(a.jumpSe); break;
    case SfxId::Damage: PlaySound(a.damageSe); break;
    case SfxId::StageChoose: PlaySound(a.stageChooseSe); break;
    case SfxId::StageDecide: PlaySound(a.stageDecideSe); break;
	case SfxId::tabclick: PlaySound(a.tabclickSe); break;
    case SfxId::Punch: PlaySound(a.punch); break;

    default: break;
    }
}

void AudioShutdown(AudioManager& a) {
    if (!a.initialized) return;

    UnloadMusicStream(a.titleBgm);
    UnloadMusicStream(a.stageBgm);
    UnloadMusicStream(a.stage3Bgm);
    UnloadMusicStream(a.chooseStageBgm);
	UnloadMusicStream(a.stage4Bgm);

    UnloadSound(a.jumpSe);
    UnloadSound(a.damageSe);
    UnloadSound(a.stageChooseSe);
    UnloadSound(a.stageDecideSe);
    UnloadSound(a.tabclickSe);
    UnloadSound(a.punch);

    CloseAudioDevice();
    a.initialized = false;
}