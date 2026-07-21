#pragma once
#include "raylib.h"

// ================================================================
// AudioManager.h の役割
// ---------------------------------------------------------------
// ・BGM/SEリソースと音量フェード状態をまとめて管理する。
// ・タイトル画面1とタイトル画面2で別々のBGMを再生する。
// ・ステージクリア後の専用BGMと、おじさん会話SEを管理する。
// ================================================================

enum class SfxId {
    Jump,
    Damage,
    StageChoose,
    StageDecide,
    DialogEnter,
    tabclick,
    Punch,
    OjisanTalk
};

struct AudioManager {
    // タイトル画面1「やさしいゲーム」用。
    Music title1Bgm{};

    // タイトル画面2「イージーなんていわせない」用。
    // 既存の assets/BGM/titleBgm.mp3 を使用する。
    Music title2Bgm{};

    Music stageBgm{};
    Music stageClearBgm{};
    Music stage3Bgm{};
    Music chooseStageBgm{};
    Music stage4Bgm{};

    Sound jumpSe{};
    Sound damageSe{};
    Sound stageChooseSe{};
    Sound stageDecideSe{};

    // ステージクリア会話中のENTER専用SE。
    Sound dialogEnterSe{};

    Sound tabclickSe{};
    Sound punch{};
    Sound ojisanTalkSe{};

    float title1MaxVol = 0.30f;
    float title2MaxVol = 0.30f;
    float stageMaxVol = 0.35f;
    float stageClearMaxVol = 0.35f;
    float stage3MaxVol = 0.35f;
    float chooseStageMaxVol = 0.35f;
    float stage4MaxVol = 0.35f;
    float editorBgmVol = 0.12f;
    float fadeSpeed = 1.2f;

    float title1Vol = 0.0f;
    float title2Vol = 0.0f;
    float stageVol = 0.0f;
    float stageClearVol = 0.0f;
    float stage3Vol = 0.0f;
    float chooseStageVol = 0.0f;
    float stage4Vol = 0.0f;

    float ojisanTalkSeVolume = 0.45f;
    float dialogEnterSeVolume = 0.60f;

    // 任意追加音源の読み込み状態。
    // falseの場合もゲーム本体は動作する。
    bool title1BgmLoaded = false;
    bool stageClearBgmLoaded = false;
    bool ojisanTalkSeLoaded = false;
    bool dialogEnterSeLoaded = false;

    bool initialized = false;
};

bool AudioInit(AudioManager& a);

void AudioUpdate(
    AudioManager& a,
    bool playTitle1Bgm,
    bool playTitle2Bgm,
    bool playStageBgm,
    bool playStageClearBgm,
    bool playStage3Bgm,
    bool playStage4Bgm,
    bool playChooseStageBgm,
    bool inEditor,
    bool inUITab,
    float dt
);

void AudioPlaySfx(AudioManager& a, SfxId id);
void AudioStopSfx(AudioManager& a, SfxId id);
void AudioShutdown(AudioManager& a);