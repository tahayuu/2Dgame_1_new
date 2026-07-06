#include <filesystem>
#include "raylib.h"
#include "GameObjects.h"
#include"Stage.h"
#include"StageTypes.h"
#include"StageCollision.h"
#include"StageHazard.h"
#include"StageDraw.h"
#include "ChengeStage.h"
#include <string>
#include"EnemyManager.h"
#include"ItemManager.h"
#include "OjisanVisual.h"
#include "DialogManager.h"
#include "GameEvents.h"
#include "OjisanDialog.h"
#include "StageEditor.h"
#include "CameraController.h"  
#include "AudioManager.h"
#include "GameState.h"
#include "TitleScene.h"
#include "Player.h"
#include "PlayerVisual.h"      
#include "SpriteDatabase.h"

int main() {

	GameState gameState = GameState::START;
	GameState pausePrevState = GameState::PLAYING; // ポーズ前の状態を保持
	int pauseSelectIdx = 0; // 0=ゲームにもどる, 1=タイトルに戻る, 2=ゲーム終了
	bool pauseTransitionBlocked = false; // ポーズ状態遷移直後の入力ブロック

	AudioManager audio;// オーディオ管理用構造体

    float deadTime = 0.0f;
    float deadingTime = 0.0f;
    const float deadDelay = 0.3f;
    const float deadingstate = 0.8f;
    const int screenWidth = 1280;
    const int screenHeight = 720;

    
    StageVisual sv;
    OjisanVisual ojisan;

    InitWindow(screenWidth, screenHeight, "step1_TeST");

    namespace fs = std::filesystem;

// 実行場所がどこでも、assets があるゲームルートを探索
// 優先順位:
// 1) <親>/2Dgame_1/assets（開発環境）
// 2) <実行フォルダ>/assets（配布zip）
auto FindGameRoot = []() -> fs::path {
    fs::path p = fs::path(GetApplicationDirectory());
    fs::path fallbackDistRoot;

    for (int i = 0; i < 8; ++i) {
        const fs::path devRoot = p / "2Dgame_1";
        if (fs::exists(devRoot / "assets")) {
            return devRoot;
        }

        if (fallbackDistRoot.empty() && fs::exists(p / "assets")) {
            fallbackDistRoot = p;
        }

        if (!p.has_parent_path()) break;
        p = p.parent_path();
    }

    if (!fallbackDistRoot.empty()) {
        return fallbackDistRoot;
    }

    return fs::path(GetApplicationDirectory());
};

const fs::path gameRoot = FindGameRoot();
ChangeDirectory(gameRoot.string().c_str()); // assets, ojisan_lines.text の相対パス基準

const fs::path stageDir = gameRoot / "data" / "stages";
fs::create_directories(stageDir);

AudioInit(audio);// オーディオ初期化

auto GetStageSaveName = [](int stageId) -> std::string {
    return "stage_edit_" + std::to_string(stageId);
};

auto ResolveStageCsvPath = [&](const std::string& baseName) -> std::string {
    return (stageDir / (baseName + ".csv")).string();
};

auto ResolveStageSaveBasePath = [&](const std::string& baseName) -> std::string {
    return (stageDir / baseName).string();
};

StageVisualLoad(sv);
ojisan.Load();
EnemyLoadTextures();
SpriteDatabase::Load(); // ← 追加：SpriteIdごとの切り出し済みPNG（assets/images/sprites/*.png）を読み込む

Texture2D titleBg = LoadTexture("assets/images/stage/background/title4.png");
Texture2D stage1Bg = LoadTexture("assets/images/stage/background/stage1.png");
Texture2D stage2Bg = LoadTexture("assets/images/stage/background/stage2.png");



    // DialogManager のロードを先に行う
    DialogManager::Instance().LoadFromFile("ojisan_lines.text");

    // ダイアログから全テキストを取得して結合
    std::string allDialogText;
    for (const auto& key : DialogManager::Instance().Keys()) {
        std::string line = DialogManager::Instance().Get(key, "");
        allDialogText += line;
        allDialogText += "\n";
    }

    // UIテキストも追加（既存の uiChars8 に追記）
    const char8_t* uiChars8 = u8"モードせんたく\n"
        u8"イージーモード\n"
        u8"デバッグモード\n"
        u8"ステージ１\n"
        u8"ステージ２\n"
        u8"ステージ3\n"
        u8"YOU DIED\n"
        u8"PRESS ENTER / SPACE\n"
        u8"ゲームにもどる\n"
        u8"タイトルにもどる\n"
        u8"ゲームしゅうりょう\n"
        u8"\n"    
        ;

     
    std::string uiText = reinterpret_cast<const char*>(uiChars8);
    allDialogText += uiText;

    // Editor UI text (for font codepoints)s
    const char8_t* editorChars8 =
        u8"\u666E\u901A\u5E8C\u4E0A\u5074\u306E"           // 普通床奥側の
        u8"\u3068\u3052\u89E6\u58CA\u4E0B"                  // とげ触壊下
        u8"\u30D6\u30ED\u30C3\u30C1\u305B\u308B"            // ブロック せる
        u8"\u30A8\u30EC\u30D9\u30FC\u30BF"                  // エレベータ
        u8"\u91CD\u529B\u30DC\u30BF\u30F3"                  // 重力ボタン
        u8"\u30A2\u30A4\u30C6\u30E0"                        // アイテム
        u8"\u6C37\u79FB\u52D5\u4F4E\u4E0A\u6607"            // 氷移動低上昇
        u8"\u30B8\u30E3\u30F3\u30D7\u53F0\u7832"            // ジャンプ台砲
        u8"\u30AB\u30FC\u30BD\u30EB\u78C1\u77F3"            // カーソル磁石
        u8"\u5439\u98DB\u3070\u3057\u58C1"                  // 吹飛ばし壁
        u8"\u308F\u304B\u308C\u5186\u8ECC\u9053"            // わかれ円軌道
        u8"\u4E57\u52D5\u304F"                              // 乗動く
        u8"YX\u62E1\u5F35\u304C\u308B"                     // YX拡張がる
        u8"\u8FFD\u5C3E\u56DE\u8EE2\u9244\u7403"            // 追尾回転鉄球
        u8"\u30EF\u30A4\u30E4\u8EE2"                        // ワイヤー転
        u8"\u843D\u5F80\u5FA9\u900F\u660E"                  // 落往復透明
        u8"\u30B9\u30A4\u30C3\u30C1\u6587\u5B57"            // スイッチ文字
        u8"\u51FA\u53E3\u30C9\u30A2\u884C\u304D"            // 出口ドア行き
        u8"\u30EA\u30B9\u30DD\u30F3"                        // リスポン
        u8"\u9078\u629E\u4E2D\u4FDD\u5B58\u3057\u307E\u305F\uFF01" // 選択中保存しました！
        u8"\u914D\u7F6E\u524A\u9664\u8AAD\u8FBC"            // 配置削除読込
        u8"\u5143\u306B\u623B\u3059\u958B\u59CB"            // 元に戻す開始
        u8"\u30D7\u30EC\u623B\n"                           // プレイ戻
        u8"\u30A3\u30C6\u30C7"                              // ィテデ（エディタステージ用）
        u8"\u5DE6\u53F3\u5E45\u9AD8\u753B\u9762\u64CD\u4F5C\u53D6\u6D88"; // 左右幅高画面操作取消
    allDialogText += reinterpret_cast<const char*>(editorChars8);

    // @キーシステム & コメントブロック入力用:
    // 半角ASCII印刷可能文字（@a-zA-Z0-9 など DrawTextEx で表示するために必要）
    allDialogText +=
        " !\"#$%&'()*+,-./"
        "0123456789:;<=>?@"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "[\\]^_`"
        "abcdefghijklmnopqrstuvwxyz"
        "{|}~";

    // コメントブロックエディタ用ヒント文字列（@キー名入力・プレビュー表示）
    const char8_t* commentHintChars8 =
        u8"\u81EA\u7531\u30C6\u30AD\u30B9\u30C8\u307E\u305F\u306F\u540D\u3067\u5165\u529B"  // 自由テキストまたは名で入力
        u8"\uFF08\u30AF\u30EA\u30C3\u30AF\u3057\u3066\u7DE8\u96C6\uFF09"                    // （クリックして編集）
        u8"\uFF08\u304C\u898B\u3064\u304B\u308A\u307E\u305B\u3093\uFF09";                   // （が見つかりません）
    allDialogText += reinterpret_cast<const char*>(commentHintChars8);

    // 結合したテキストからコードポイントを生成
    int ojisanCpCount = 0;
    int* ojisanCps = LoadCodepoints(allDialogText.c_str(), &ojisanCpCount);


    // フォントサイズを64に増やして高解像度テクスチャを生成
    Font ojisanFont = LoadFontEx("assets/images/font/YDWaosagi.otf", 64, ojisanCps, ojisanCpCount);
    if (ojisanFont.texture.id == 0) {
   
    }
    else {
        GenTextureMipmaps(&ojisanFont.texture);
        SetTextureFilter(ojisanFont.texture, TEXTURE_FILTER_BILINEAR);

    }
    UnloadCodepoints(ojisanCps);

    int jpCpCount = 0;
    int* jpCps = LoadCodepoints(allDialogText.c_str(), &jpCpCount);
    Font jpFont = LoadFontEx("assets/images/font/titlefont.ttf", 64, jpCps, jpCpCount);
    if (jpFont.texture.id == 0) {
     
    }
    else {
        GenTextureMipmaps(&jpFont.texture);
        SetTextureFilter(jpFont.texture, TEXTURE_FILTER_BILINEAR);

    }
    UnloadCodepoints(jpCps);

    // おじさんのデフォルトフォントを YDWaosagi.otf に設定
    ojisan.SetFont(ojisanFont);

    SetTargetFPS(60);
    Stage* stagePtr = new Stage{};
    Stage& stage = *stagePtr;
    StageEditor stageEditor;
    EnemyManager enemyManager;
    enemyManager.Init();
    ItemManager itemManager;
    itemManager.Init();
    itemManager.Load();
    Font textFont = jpFont;
    TitleScene titleScene;
    TitleSceneInit(titleScene, titleBg, textFont, audio);

    int currentStage = 1;
    stage.fallingTexts[0].Init({ 200.0f,220.0f }, jpFont);// フォント情報で幅/高さを設定
    stage.fallingTextsInit[0] = stage.fallingTexts[0];

    //プレイヤー
    const int PLAYER_W = 60;
    const int PLAYER_H = 70;

    PlayerState playerState;
    PlayerStateInit(playerState);
    Rectangle& player = playerState.rect;
    Vector2& velocity = playerState.velocity;
    bool& onGround = playerState.onGround;
    bool& onTop = playerState.onTop;
    bool& isCraneGrabbed = playerState.isCraneGrabbed;
    Vector2& respawn = playerState.respawn;
    int& deaths = playerState.deaths;
    bool& isOjisanPunchDeath = playerState.isOjisanPunchDeath;

	const float stageWidth = 15000.0f;
	const float spikeW = 15.0f;

    //横スクロール
    Camera2D camera = { 0 };/*初期化Raylivが用意している構造体
    struct Camera2D {
       Vector2 offset;   画面のどこに映すか
       Vector2 target;   世界のどこを見るか
       float rotation;   回転
       float zoom;       拡大率
   };*/
    camera.offset = { screenWidth / 2.0f,screenHeight / 2.0f };//画面のどこに target を表示するか（画面座標)
    camera.target = { playerState.rect.x + playerState.rect.width / 2 ,playerState.rect.y + playerState.rect.height / 2 };//世界のどこを見るか（世界座標）
    camera.zoom = 1.0f;//拡大率

    float editorExitInvTimer = 0.0f;        // エディタ終了後の無敵時間
    const float editorExitInvDuration = 2.0f; // 無敵秒数
    float editorKeyBlockTimer = 0.0f;       // エディタ切替直後のキー入力無視時間
    const float editorKeyBlockDuration = 0.3f; // キー無視秒数

    auto ResetPlayerToDefault = [&](float invincibleSec = -1.0f) {
        player = { 100.0f, 500.0f, (float)PLAYER_W, (float)PLAYER_H };
        velocity = { 0.0f, 0.0f };
        if (invincibleSec >= 0.0f) {
            editorExitInvTimer = invincibleSec;
        }
    };

    auto LoadSelectedStage = [&]() {
        StageClear(stage);
        ojisan.showPunch = false;  // パンチフラグをリセット

        switch (titleScene.selectStage) {
        case 0:
            StageInit_1(stage, enemyManager, itemManager);
            currentStage = 1;
            break;
        case 1:
            StageInit_debug(stage, enemyManager, itemManager);
            currentStage = 99;
            break;
        case 2:
            StageInit_2(stage, enemyManager, itemManager);
            currentStage = 2;
            break;
        case 3:
            StageInit_3(stage, enemyManager, itemManager);
            currentStage = 3;
            break;
        case 4:
            StageInit_choseStage(stage, enemyManager);
            currentStage = 0;
            break;
        case 5: {
            StageEditor tempEd;
            tempEd.savePath = ResolveStageSaveBasePath("stage_editor_output");
            std::string csvPath = ResolveStageCsvPath("stage_editor_output");
            if (FileExists(csvPath.c_str()) && EditorLoadCSV(tempEd, csvPath.c_str())) {
                EditorExportToStage(tempEd, stage, enemyManager);
            }
            currentStage = 100;
            break;
        }
        default:
            break;
        }

        // 保存済みエディタデータがある場合はステージに上書き適用
        // （currentStage == 100 はエディタステージ自身なのでスキップ）
        if (currentStage != 100) {
            const std::string stageBaseName = GetStageSaveName(currentStage);
            std::string csvPath = ResolveStageCsvPath(stageBaseName);
            StageEditor tempEd;
            if (FileExists(csvPath.c_str()) && EditorLoadCSV(tempEd, csvPath.c_str())) {
                EditorExportToStage(tempEd, stage, enemyManager);
            }
        }

        itemManager.saveItemsInit();
        enemyManager.saveEnemiesInit();
        enemyManager.RestorInitialEnemies();
        itemManager.RestorInitialItems();
    };

    auto EnterStageEditor = [&]() {
        stageEditor.savePath = (currentStage == 100)
            ? ResolveStageSaveBasePath("stage_editor_output")
            : ResolveStageSaveBasePath(GetStageSaveName(currentStage));
        EditorInit(stageEditor, screenWidth, screenHeight, jpFont);
        if (stageEditor.objects.empty()) {
            EditorImportFromStage(stageEditor, stage);
        }
        stageEditor.camera.target = {
            player.x + player.width / 2.0f,
            player.y + player.height / 2.0f
        };
        gameState = GameState::EDITOR;
        editorKeyBlockTimer = editorKeyBlockDuration; // キー入力を一時的にブロック
    };

    while (!WindowShouldClose()) {

        float dt = GetFrameTime();


		const bool playTitleBgm = (gameState == GameState::START);
		const bool inStageScene =
			(gameState == GameState::PLAYING ||
			 gameState == GameState::DEADING_SCREEN ||
			 gameState == GameState::DEAD_SCREEN ||
			 gameState == GameState::PAUSE ||
			 gameState == GameState::EDITOR);  // エディタ中もステージBGMを流す
		const bool playStageBgm = inStageScene && (currentStage == 1 || currentStage == 2);
		const bool playStage3Bgm = inStageScene && (currentStage == 3);
		const bool playChooseStageBgm = inStageScene && (currentStage == 0);

		// エディタモード中、またはUIパネルが開いている場合は音量を下げる
		bool inEditor = (gameState == GameState::EDITOR);
		bool inUITab = (gameState == GameState::EDITOR) && 
					   (stageEditor.propSelectedIdx >= 0 || stageEditor.enemyMenuOpen);

		AudioUpdate(audio, playTitleBgm, playStageBgm, playStage3Bgm, playChooseStageBgm, inEditor, inUITab, dt);// オーディオ更新

        // ===== エディタモード: BeginDrawing より前に完結させる =====
        if (gameState == GameState::EDITOR) {
            // エディタ切替直後のキー入力無視タイマー更新
            if (editorKeyBlockTimer > 0.0f) {
                editorKeyBlockTimer -= dt;
                if (editorKeyBlockTimer < 0.0f) editorKeyBlockTimer = 0.0f;
            }

            EditorUpdate(stageEditor, dt);

            // F5: エディタ内容を適用してプレイテスト開始
            if (IsKeyPressed(KEY_C)) {
                EditorSaveCSV(stageEditor, (stageEditor.savePath + ".csv").c_str());
                EditorExportToStage(stageEditor, stage, enemyManager);
                StageReset(stage);
                HazardReset(stage);
                enemyManager.RestorInitialEnemies();
                itemManager.RestorInitialItems();
                if (stage.hasRespawnPoint) {
                    player = { stage.respawnPoint.x, stage.respawnPoint.y, (float)PLAYER_W, (float)PLAYER_H };
                    respawn = stage.respawnPoint;
                } else {
                    ResetPlayerToDefault();
                    respawn = { 100.0f, 500.0f };
                }
                velocity = { 0.0f, 0.0f };
                onGround = false;
                editorExitInvTimer = editorExitInvDuration;
                gameState = GameState::PLAYING;
            }

            if ((IsKeyPressed(KEY_V) || IsKeyPressed(KEY_F1)) && editorKeyBlockTimer <= 0.0f) {
                EditorSaveCSV(stageEditor, (stageEditor.savePath + ".csv").c_str());
                stageEditor.active = false;
                editorExitInvTimer = editorExitInvDuration;
                if (stage.hasRespawnPoint) {
                    player = { stage.respawnPoint.x, stage.respawnPoint.y, (float)PLAYER_W, (float)PLAYER_H };
                    velocity = { 0.0f, 0.0f };
                    respawn = stage.respawnPoint;
                }
                gameState = GameState::PLAYING;
                editorKeyBlockTimer = editorKeyBlockDuration; // キー入力を一時的にブロック
            }

            BeginDrawing();
            ClearBackground({ 40, 40, 50, 255 });
            BeginMode2D(stageEditor.camera);
            EditorDraw(stageEditor);
            EndMode2D();
            EditorDrawUI(stageEditor);
            EndDrawing();
            continue;
        }

        // Ojisan の更新：タイトル画面以外で毎フレーム更新
        if (gameState != GameState::START) {
            ojisan.Update(dt);
        }


        DeathCause cause = DeathCause::FALL; // デフォルト値（適宜変更）

        auto RespawnPlayer = [&]() {
            deaths++;
            gameState = GameState::DEADING_SCREEN;
            deadTime = 0.0f;
            deadingTime = 0.0f;
            enemyManager.playerTouched = false;
            if (cause != DeathCause::OJISAN_PUNCH) {
                isOjisanPunchDeath = false;
            }
            for (int i = 0; i < stage.clearsCount; i++) {
                stage.clearBlocks[i].clearflag = false;
            }

            // ★ 死亡ブロック(TRAP)死亡時はコメントを出さない
            if (cause == DeathCause::TRAP) {
                return;
            }

            // OjisanContext を組み立てて拡張版を使う
            OjisanContext ctx;
            ctx.cause = cause;
            ctx.stageName = "stage" + std::to_string(currentStage);
            ctx.totalDeaths = deaths;

            std::string line = GetOjiSanLine(ctx);

            if (!enemyManager.touchedEnemyDialogKey.empty() &&
                DialogManager::Instance().Has(enemyManager.touchedEnemyDialogKey)) {
                line = DialogManager::Instance().Get(enemyManager.touchedEnemyDialogKey, "");
            }
            ojisan.SetFont(ojisanFont);
            ojisan.TriggerMessage(line, 3.0f, &ojisanFont);
        };

        auto RestartGame = [&]() {
            player.x = respawn.x;
            player.y = respawn.y;
            velocity = { 0.0f,0.0f };
            onGround = false;
            onTop = false;
            stage.gravityReversed = false;
            stage.currentLayer = 0;
            isOjisanPunchDeath = false;
            ojisan.showPunch = false;  // パンチフラグをリセット

            StageReset(stage);
            HazardReset(stage);
            enemyManager.RestorInitialEnemies();
            itemManager.RestorInitialItems();
            gameState = GameState::PLAYING;
            };

		//プレイ画面
		 if (gameState == GameState::PLAYING) {

			 //ポーズメニューを開く
			 if (IsKeyPressed(KEY_TAB)) {
				 pausePrevState = gameState;
				 pauseSelectIdx = 0;
				 pauseTransitionBlocked = true;
				 gameState = GameState::PAUSE;
				 AudioPlaySfx(audio, SfxId::tabclick);
			 }

			if (editorExitInvTimer > 0.0f) {
				editorExitInvTimer -= dt;
				if (editorExitInvTimer < 0.0f) editorExitInvTimer = 0.0f;
			}
			const bool isInvincible = (editorExitInvTimer > 0.0f);

			PlayerStateUpdate(playerState, stage, enemyManager, itemManager, ojisan, camera, dt, isInvincible, audio);

			if (playerState.pendingEnterEditor) {
				EnterStageEditor();
				continue;
			}

			if (playerState.pendingDeath) {
				cause = playerState.lastDeathCause;
                if ((cause == DeathCause::ENEMY_WALKER ||
                    cause == DeathCause::ENEMY_FLYER ||
                    cause == DeathCause::ENEMY_SHOOTER) &&
                    enemyManager.touchedEnemyFromSide &&
                    enemyManager.touchedEnemyIndex >= 0 &&
                    enemyManager.touchedEnemyIndex < (int)enemyManager.enemies.size()) {
                    enemyManager.enemies[enemyManager.touchedEnemyIndex].isHit = true;
                }
				RespawnPlayer();
			}

			if (gameState != GameState::PLAYING) {
				continue;
			}

			// exitDoor判定：上向きキーでステージ遷移
			if (stage.exitDoorTriggered >= 0) {
				if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
					int targetStage = stage.exitDoors[stage.exitDoorTriggered].targetStage;
				 titleScene.selectStage = targetStage;
					LoadSelectedStage();
					ResetPlayerToDefault(1.0f);
					gameState = GameState::PLAYING;
				}
			}

			// ワープホール判定：W キーでテレポート
			if (stage.warpTriggered >= 0) {
				player.x = stage.warps[stage.warpTriggered].place.x;
				player.y = stage.warps[stage.warpTriggered].place.y;
				stage.warpTriggered = -1;
			}
		}

        //死亡硬直画面
		else if (gameState == GameState::DEADING_SCREEN) {//死亡してから少しの間、プレイヤーの操作を受け付けない状態
            deadingTime += dt;

            if (deadingTime >= deadingstate) {
                gameState = GameState::DEAD_SCREEN;
                deadTime = 0.0f;
            }
        }

        //死亡画面
        else if (gameState == GameState::DEAD_SCREEN) {
            deadTime += dt;

            if (deadTime >= deadDelay && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))) {
                RestartGame();
            }
        }

		else if (gameState == GameState::START) {
			if (TitleSceneUpdate(titleScene, gameState, dt)) {
				LoadSelectedStage();
				ResetPlayerToDefault(1.0f); // スタート開始時に1秒無敵
			}

			// V/F1 でエディタ開始（TitleScene 側に未実装のため main で補完）
			if (titleScene.isSelectingStage && (IsKeyPressed(KEY_V) || IsKeyPressed(KEY_F1))) {
				LoadSelectedStage();
				ResetPlayerToDefault();
				titleScene.isSelectingStage = false;
				EnterStageEditor();
			}
		}

        else if (gameState == GameState::Deback) {

        }

        // PAUSE状態の入力処理ブロック
        else if (gameState == GameState::PAUSE) {
            // 状態遷移直後のフレームは入力をブロック
            if (pauseTransitionBlocked) {
                pauseTransitionBlocked = false;

            } else {
    
                const int pauseMenuCount = 3;

                // 上下で選択
                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                    pauseSelectIdx = (pauseSelectIdx + pauseMenuCount - 1) % pauseMenuCount;
					AudioPlaySfx(audio, SfxId::StageChoose);
                }
                if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                    pauseSelectIdx = (pauseSelectIdx + 1) % pauseMenuCount;
                    AudioPlaySfx(audio, SfxId::StageChoose);
                }

                // ESC / TAB で再開
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_TAB)) {
                    AudioPlaySfx(audio, SfxId::tabclick);
                    pauseTransitionBlocked = true;  // ★ PAUSE→PLAYING 遷移時もブロック
                    gameState = pausePrevState;
                }

                // ENTER で決定
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    if (pauseSelectIdx == 0) {
                        // ゲームに戻る
                        AudioPlaySfx(audio, SfxId::StageDecide);
                        pauseTransitionBlocked = true;  // ★
                        gameState = pausePrevState;
                    }
                    else if (pauseSelectIdx == 1) {
                        // タイトルに戻る
                        StageClear(stage);
                        ojisan.showPunch = false;  // パンチフラグをリセット
                        enemyManager.Reset();
                        itemManager.Reset();
                        deaths = 0;
                        editorExitInvTimer = 0.0f;
                        gameState = GameState::START;
                        titleScene.isSelectingStage = false;
                    }
                    else {
                        // ゲーム終了
                        AudioShutdown(audio);

                        CloseWindow();
                        return 0;
                    }
                }
            }
        }

        // ===== 描画 =====
        BeginDrawing();

        if (gameState == GameState::START) {
            TitleSceneDraw(titleScene, screenWidth, screenHeight);
        }
        
        else if (gameState == GameState::PLAYING ||
            gameState == GameState::DEADING_SCREEN ||
            gameState == GameState::DEAD_SCREEN) {

            ClearBackground({ 135, 206, 235, 255 }); // 空色


            // ステージ1・2は森の背景を全画面表示
            if ((currentStage == 1 || currentStage == 2) && stage1Bg.id != 0) {
                DrawTexturePro(
                    stage1Bg,
                    { 0, 0, (float)stage1Bg.width, (float)stage1Bg.height },
                    { 0, 0, (float)screenWidth,    (float)screenHeight },
                    { 0, 0 }, 0.0f, WHITE);
            }
            if ((currentStage == 3) && stage2Bg.id != 0) {
                DrawTexturePro(
                    stage2Bg,
                    { 0, 0, (float)stage2Bg.width, (float)stage2Bg.height },
                    { 0, 0, (float)screenWidth,    (float)screenHeight },
                    { 0, 0 }, 0.0f, WHITE);
            }

            // カメラを
            // プレイヤーに追従させる
            if (gameState == GameState::PLAYING && !isCraneGrabbed) {
                if (stage.cameraConfig.twoLayered) {
                    // 2段構造ステージ: 地上固定→シャフト追従→地下固定
                    UpdateCamera(camera, player, stage.cameraConfig,
                        stageWidth, stage.stageHeight,
                        screenWidth, screenHeight, dt);
                }
                else {
                    // 通常ステージ: Y軸固定、X軸クランプ
                    float camTargetX = player.x + player.width / 2.0f;
                    float minCamX = (screenWidth / 2.0f) / camera.zoom;
                    if (camTargetX < minCamX) camTargetX = minCamX;
                    camera.target = { camTargetX, 400.0f };
                }
            }
			BeginMode2D(camera);
			StageDraw(stage, spikeW, player, stage.heldSpringIndex);
			enemyManager.DrawAll();
			itemManager.DrawAll();
			const bool isDeadScreen = (gameState == GameState::DEADING_SCREEN || gameState == GameState::DEAD_SCREEN);
			PlayerStateDrawWorld(playerState, editorExitInvTimer, isDeadScreen);
			EndMode2D();

            // ← ここでUIを描く（スクリーン固定）
            DrawItemUI(stage);

            // HUD：死亡回数
            std::string deathText = "Deaths: " + std::to_string(deaths);
            DrawTextEx(textFont, deathText.c_str(), { 10, 10 }, 24, 0, WHITE);

            // 死亡演出・死亡画面
            if (isDeadScreen) {
                PlayerStateDrawScreen(playerState, isDeadScreen, ojisan.punchEffect);

                unsigned char alpha = (gameState == GameState::DEADING_SCREEN)
                    ? (unsigned char)(deadingTime / deadingstate * 180.0f)
                    : 180;
                DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, alpha });

                if (gameState == GameState::DEAD_SCREEN) {
                    DrawTextEx(textFont, "YOU DIED",
                        { screenWidth / 2.0f - 120, screenHeight / 2.0f - 60 }, 72, 0, RED);
                    if (deadTime >= deadDelay) {
                        DrawTextEx(textFont, "PRESS ENTER / SPACE",
                            { screenWidth / 2.0f - 160, screenHeight / 2.0f + 40 }, 28, 0, WHITE);
                    }
                }
            }

            // おじさんをUI要素として描画（カメラ変換外）
            ojisan.Draw(screenWidth, screenHeight);
        }

        // ポーズメニュー描画（PLAYINGの描画の上に重ねる）
        if (gameState == GameState::PAUSE) {
            // 半透明オーバーレイ
            DrawRectangle(0, 0, screenWidth, screenHeight, { 0, 0, 0, 160 });

            // メニュー背景
            const float menuW = 400.0f;
            const float menuH = 300.0f; // 3項目に増やすため拡張
            const float menuX = screenWidth / 2.0f - menuW / 2.0f;
            const float menuY = screenHeight / 2.0f - menuH / 2.0f;
            DrawRectangleRounded({ menuX, menuY, menuW, menuH }, 0.12f, 8, { 20, 20, 40, 230 });
            DrawRectangleRoundedLinesEx({ menuX, menuY, menuW, menuH }, 0.12f, 8, 2, ColorAlpha(WHITE, 0.4f));

            // タイトル
            DrawTextEx(textFont, "PAUSE",
                { menuX + menuW / 2.0f - MeasureTextEx(textFont, "PAUSE", 52, 0).x / 2.0f,
                  menuY + 20.0f }, 52, 0, WHITE);

            // 選択肢
            const char* items[] = {
                reinterpret_cast<const char*>(u8"ゲームにもどる"),
                reinterpret_cast<const char*>(u8"タイトルにもどる"),
                reinterpret_cast<const char*>(u8"ゲームしゅうりょう"),
            };
            for (int i = 0; i < 3; i++) {
                Color col = (i == pauseSelectIdx) ? YELLOW : LIGHTGRAY;
                float iy = menuY + 100.0f + i * 52.0f;
                float tw = MeasureTextEx(textFont, items[i], 32, 0).x;
                if (i == pauseSelectIdx) {
                    DrawTextEx(textFont, ">",
                        { menuX + menuW / 2.0f - tw / 2.0f - 28.0f, iy }, 32, 0, YELLOW);
                }
                DrawTextEx(textFont, items[i],
                    { menuX + menuW / 2.0f - tw / 2.0f, iy }, 32, 0, col);
            }

            // 操作ヒント
            DrawTextEx(textFont, "TAB/ESC: resume",
                { menuX + menuW / 2.0f - MeasureTextEx(textFont, "TAB/ESC: resume", 18, 0).x / 2.0f,
                  menuY + menuH - 28.0f }, 18, 0, GRAY);
        }

        EndDrawing();
    }

    // --- クリーンアップ ---
    enemyManager.Reset();
    itemManager.Reset();
    itemManager.Unload();
    EnemyUnloadTextures(); // 敵のテクスチャ解放
    SpriteDatabase::Unload(); // ← 追加：スプライトアトラスの解放
    PlayerStateUnload(playerState);
    StageVisualUnload(sv);
    ojisan.Unload();
    UnloadFont(ojisanFont);
    UnloadFont(jpFont);
    TitleSceneUnload(titleScene);
    if (stage1Bg.id != 0) UnloadTexture(stage1Bg);
    if (stage2Bg.id != 0) UnloadTexture(stage2Bg);
    AudioShutdown(audio);
    CloseWindow();

    return 0;
}
