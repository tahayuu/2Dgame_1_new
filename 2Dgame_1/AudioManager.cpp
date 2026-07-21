#include "AudioManager.h"

// ================================================================
// AudioManager.cpp の役割
// ---------------------------------------------------------------
// ・BGMを目標音量へ近づけ、場面間をクロスフェードさせる。
// ・タイトル画面1とタイトル画面2のBGMを個別に管理する。
// ・おじさんの文字送り中に使用する短い会話SEを管理する。
// ================================================================

namespace {
    float Approach(float current, float target, float delta)
    {
        if (current < target) {
            current += delta;
            if (current > target) current = target;
        }
        else if (current > target) {
            current -= delta;
            if (current < target) current = target;
        }

        return current;
    }

    void StartMusicWhenNeeded(Music music, float targetVolume, float currentVolume)
    {
        if ((targetVolume > 0.0f || currentVolume > 0.001f) &&
            !IsMusicStreamPlaying(music)) {
            PlayMusicStream(music);
        }
    }

    void StopMusicWhenSilent(Music music, float targetVolume, float currentVolume)
    {
        if (currentVolume <= 0.001f && targetVolume <= 0.0f &&
            IsMusicStreamPlaying(music)) {
            StopMusicStream(music);
        }
    }
}

bool AudioInit(AudioManager& a)
{
    InitAudioDevice();

    // タイトル1専用BGM。未配置の場合はタイトル2用BGMを代わりに使用する。
    const char* title1BgmPath = "assets/BGM/title1Bgm.mp3";
    a.title1BgmLoaded = FileExists(title1BgmPath);
    if (a.title1BgmLoaded) {
        a.title1Bgm = LoadMusicStream(title1BgmPath);
        a.title1Bgm.looping = true;
    }

    // 既存タイトルBGMはタイトル画面2用として使用する。
    a.title2Bgm = LoadMusicStream("assets/BGM/titleBgm.mp3");
    a.stageBgm = LoadMusicStream("assets/BGM/stage1.mp3");

    const char* stage1ClearBgmPath = "assets/BGM/stage1Clear.mp3";
    a.stage1ClearBgmLoaded = FileExists(stage1ClearBgmPath);
    if (a.stage1ClearBgmLoaded) {
        a.stage1ClearBgm = LoadMusicStream(stage1ClearBgmPath);
        a.stage1ClearBgm.looping = true;
    }

    a.stage3Bgm = LoadMusicStream("assets/BGM/stage3.mp3");
    a.chooseStageBgm = LoadMusicStream("assets/BGM/chooseStage.mp3");
    a.stage4Bgm = LoadMusicStream("assets/BGM/stage4.mp3");

    a.jumpSe = LoadSound("assets/SE/jump.wav");
    a.damageSe = LoadSound("assets/SE/damage.wav");
    a.stageChooseSe = LoadSound("assets/SE/stageChoose.wav");
    a.stageDecideSe = LoadSound("assets/SE/stageDecide.wav");

    // ステージ1クリア会話でENTERを押したときの専用SE。
    // ファイルが未配置の場合は、従来のstageDecideSeへフォールバックする。
    const char* dialogEnterSePath = "assets/SE/dialogEnter.mp3";
    a.dialogEnterSeLoaded = FileExists(dialogEnterSePath);
    if (a.dialogEnterSeLoaded) {
        a.dialogEnterSe = LoadSound(dialogEnterSePath);
        SetSoundVolume(a.dialogEnterSe, a.dialogEnterSeVolume);
    }

    a.tabclickSe = LoadSound("assets/SE/tabclick.wav");
    a.punch = LoadSound("assets/SE/punch_1.wav");

    // 会話SEは任意追加。短い「ポポポ」系の音を想定する。
    const char* ojisanTalkSePath = "assets/SE/ojisanTalk.wav";
    a.ojisanTalkSeLoaded = FileExists(ojisanTalkSePath);
    if (a.ojisanTalkSeLoaded) {
        a.ojisanTalkSe = LoadSound(ojisanTalkSePath);
        SetSoundVolume(a.ojisanTalkSe, a.ojisanTalkSeVolume);
    }

    a.title2Bgm.looping = true;
    a.stageBgm.looping = true;
    a.stage3Bgm.looping = true;
    a.chooseStageBgm.looping = true;
    a.stage4Bgm.looping = true;

    a.title1Vol = 0.0f;
    a.title2Vol = 0.0f;
    a.stageVol = 0.0f;
    a.stage1ClearVol = 0.0f;
    a.stage3Vol = 0.0f;
    a.chooseStageVol = 0.0f;
    a.stage4Vol = 0.0f;

    if (a.title1BgmLoaded) SetMusicVolume(a.title1Bgm, a.title1Vol);
    SetMusicVolume(a.title2Bgm, a.title2Vol);
    SetMusicVolume(a.stageBgm, a.stageVol);
    if (a.stage1ClearBgmLoaded) SetMusicVolume(a.stage1ClearBgm, a.stage1ClearVol);
    SetMusicVolume(a.stage3Bgm, a.stage3Vol);
    SetMusicVolume(a.chooseStageBgm, a.chooseStageVol);
    SetMusicVolume(a.stage4Bgm, a.stage4Vol);

    a.initialized = true;
    return true;
}

void AudioUpdate(
    AudioManager& a,
    bool playTitle1Bgm,
    bool playTitle2Bgm,
    bool playStageBgm,
    bool playStage1ClearBgm,
    bool playStage3Bgm,
    bool playStage4Bgm,
    bool playChooseStageBgm,
    bool inEditor,
    bool inUITab,
    float dt
) {
    if (!a.initialized) return;

    const float volumeScale = (inEditor || inUITab) ? a.editorBgmVol : 1.0f;

    // タイトル1専用音源がない場合は、タイトル2用BGMを代わりに鳴らす。
    const bool useTitle1Bgm = playTitle1Bgm && a.title1BgmLoaded;
    const bool fallbackTitle1ToTitle2 = playTitle1Bgm && !a.title1BgmLoaded;

    const bool useStage1ClearBgm = playStage1ClearBgm && a.stage1ClearBgmLoaded;
    const bool fallbackToStageBgm = playStage1ClearBgm && !a.stage1ClearBgmLoaded;

    const float targetTitle1 = useTitle1Bgm ? a.title1MaxVol * volumeScale : 0.0f;
    const float targetTitle2 = (playTitle2Bgm || fallbackTitle1ToTitle2)
        ? a.title2MaxVol * volumeScale
        : 0.0f;
    const float targetStage = (playStageBgm || fallbackToStageBgm)
        ? a.stageMaxVol * volumeScale
        : 0.0f;
    const float targetStage1Clear = useStage1ClearBgm
        ? a.stage1ClearMaxVol * volumeScale
        : 0.0f;
    const float targetStage3 = playStage3Bgm ? a.stage3MaxVol * volumeScale : 0.0f;
    const float targetStage4 = playStage4Bgm ? a.stage4MaxVol * volumeScale : 0.0f;
    const float targetChooseStage = playChooseStageBgm
        ? a.chooseStageMaxVol * volumeScale
        : 0.0f;

    if (a.title1BgmLoaded) StartMusicWhenNeeded(a.title1Bgm, targetTitle1, a.title1Vol);
    StartMusicWhenNeeded(a.title2Bgm, targetTitle2, a.title2Vol);
    StartMusicWhenNeeded(a.stageBgm, targetStage, a.stageVol);
    if (a.stage1ClearBgmLoaded) {
        StartMusicWhenNeeded(a.stage1ClearBgm, targetStage1Clear, a.stage1ClearVol);
    }
    StartMusicWhenNeeded(a.stage3Bgm, targetStage3, a.stage3Vol);
    StartMusicWhenNeeded(a.stage4Bgm, targetStage4, a.stage4Vol);
    StartMusicWhenNeeded(a.chooseStageBgm, targetChooseStage, a.chooseStageVol);

    const float step = a.fadeSpeed * dt;
    a.title1Vol = Approach(a.title1Vol, targetTitle1, step);
    a.title2Vol = Approach(a.title2Vol, targetTitle2, step);
    a.stageVol = Approach(a.stageVol, targetStage, step);
    a.stage1ClearVol = Approach(a.stage1ClearVol, targetStage1Clear, step);
    a.stage3Vol = Approach(a.stage3Vol, targetStage3, step);
    a.stage4Vol = Approach(a.stage4Vol, targetStage4, step);
    a.chooseStageVol = Approach(a.chooseStageVol, targetChooseStage, step);

    if (a.title1BgmLoaded) SetMusicVolume(a.title1Bgm, a.title1Vol);
    SetMusicVolume(a.title2Bgm, a.title2Vol);
    SetMusicVolume(a.stageBgm, a.stageVol);
    if (a.stage1ClearBgmLoaded) SetMusicVolume(a.stage1ClearBgm, a.stage1ClearVol);
    SetMusicVolume(a.stage3Bgm, a.stage3Vol);
    SetMusicVolume(a.stage4Bgm, a.stage4Vol);
    SetMusicVolume(a.chooseStageBgm, a.chooseStageVol);

    if (a.title1BgmLoaded) UpdateMusicStream(a.title1Bgm);
    UpdateMusicStream(a.title2Bgm);
    UpdateMusicStream(a.stageBgm);
    if (a.stage1ClearBgmLoaded) UpdateMusicStream(a.stage1ClearBgm);
    UpdateMusicStream(a.stage3Bgm);
    UpdateMusicStream(a.stage4Bgm);
    UpdateMusicStream(a.chooseStageBgm);

    if (a.title1BgmLoaded) StopMusicWhenSilent(a.title1Bgm, targetTitle1, a.title1Vol);
    StopMusicWhenSilent(a.title2Bgm, targetTitle2, a.title2Vol);
    StopMusicWhenSilent(a.stageBgm, targetStage, a.stageVol);
    if (a.stage1ClearBgmLoaded) {
        StopMusicWhenSilent(a.stage1ClearBgm, targetStage1Clear, a.stage1ClearVol);
    }
    StopMusicWhenSilent(a.stage3Bgm, targetStage3, a.stage3Vol);
    StopMusicWhenSilent(a.stage4Bgm, targetStage4, a.stage4Vol);
    StopMusicWhenSilent(a.chooseStageBgm, targetChooseStage, a.chooseStageVol);
}

void AudioPlaySfx(AudioManager& a, SfxId id)
{
    if (!a.initialized) return;

    switch (id) {
    case SfxId::Jump: PlaySound(a.jumpSe); break;
    case SfxId::Damage: PlaySound(a.damageSe); break;
    case SfxId::StageChoose: PlaySound(a.stageChooseSe); break;
    case SfxId::StageDecide: PlaySound(a.stageDecideSe); break;
    case SfxId::DialogEnter:
        if (a.dialogEnterSeLoaded) {
            PlaySound(a.dialogEnterSe);
        }
        else {
            PlaySound(a.stageDecideSe);
        }
        break;
    case SfxId::tabclick: PlaySound(a.tabclickSe); break;
    case SfxId::Punch: PlaySound(a.punch); break;
    case SfxId::OjisanTalk:
        if (a.ojisanTalkSeLoaded) PlaySound(a.ojisanTalkSe);
        break;
    }
}

void AudioStopSfx(AudioManager& a, SfxId id)
{
    if (!a.initialized) return;

    switch (id) {
    case SfxId::OjisanTalk:
        if (a.ojisanTalkSeLoaded && IsSoundPlaying(a.ojisanTalkSe)) {
            StopSound(a.ojisanTalkSe);
        }
        break;
    default:
        break;
    }
}

void AudioShutdown(AudioManager& a)
{
    if (!a.initialized) return;

    if (a.title1BgmLoaded) UnloadMusicStream(a.title1Bgm);
    UnloadMusicStream(a.title2Bgm);
    UnloadMusicStream(a.stageBgm);
    if (a.stage1ClearBgmLoaded) UnloadMusicStream(a.stage1ClearBgm);
    UnloadMusicStream(a.stage3Bgm);
    UnloadMusicStream(a.chooseStageBgm);
    UnloadMusicStream(a.stage4Bgm);

    UnloadSound(a.jumpSe);
    UnloadSound(a.damageSe);
    UnloadSound(a.stageChooseSe);
    UnloadSound(a.stageDecideSe);
    if (a.dialogEnterSeLoaded) UnloadSound(a.dialogEnterSe);
    UnloadSound(a.tabclickSe);
    UnloadSound(a.punch);
    if (a.ojisanTalkSeLoaded) UnloadSound(a.ojisanTalkSe);

    CloseAudioDevice();

    a.title1BgmLoaded = false;
    a.stage1ClearBgmLoaded = false;
    a.ojisanTalkSeLoaded = false;
    a.dialogEnterSeLoaded = false;
    a.initialized = false;
}