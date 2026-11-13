#include "Stage.h"

// StageInit_3.cpp の役割: ステージ3（縦構造あり）の初期配置を定義する。
// 目的: 地上/地下レイヤーを含む地形とギミック配置を構築する。
void StageInit_3(Stage& stage, EnemyManager& enemyManager, ItemManager& itemManager) {

    // ステージ3用テクスチャ（パスは必要に応じて変更）
    StageThemeLoadAll(stage.theme,
        "assets/images/stage/stage_3/ground1.png",
        "assets/images/stage/stage_3/ground2.png",
        "assets/images/stage/stage_3/ground3.png",
        "assets/images/stage/stage_3/ground4.png",
        50.0f);

    /*StageThemeLoadObjectTextures(
        stage.theme,
        "assets/images/stage/stage_1/itemblock.png",
        "assets/images/stage/stage_1/normalblock.png",
        "assets/images/items/Arrow.png",
        "assets/images/stage/stage_1/Bullet.png",
        "assets/images/stage/stage_1/ActionButtn_off.png",
        "assets/images/stage/stage_1/ActionButtn_on.png",
		"assets/images/stage/stage_3/Buttom_break.png"
        
    );*/


    // =====================================================
    //  地上レイヤー  (Y: 0 ～ 720)
    //  穴: x=700~900 に地下への縦穴
    // =====================================================

    stage.platforms[0] = { 0.0f,    650.0f,  200.0f, 70.0f };   // 左の地面
	stage.platforms[21] = { 350.0f,    650.0f,  650.0f, 70.0f };  // 左の地面
    stage.platforms[1] = { 1200.0f,  650.0f, 3500.0f, 70.0f };   // 右の地面（x=4400 まで）
    stage.platforms[2] = { 0.0f, 0.0f,  50.0f, 650.0f };   // 左の壁
    stage.platforms[3] = { 0.0f, 0.0f,  2000.0f, 30.0f };   // 天井
    stage.platforms[4] = { 950.0f, 400.0f,  50.0f, 250.0f };   // 土管
    stage.platforms[5] = { 1200.0f, 400.0f,  50.0f, 250.0f };   // 土管
    stage.platforms[16] = { 2000.0f, 0.0f,  50.0f, 1000.0f };// 右のかべ
    stage.platforms[17] = { 1400.0f, 550.0f,  100.0f, 20.0f };


    stage.cursorPlatforms[0] = {
    { 800.0f, 550.0f, 100.0f, 20.0f },  // rect: x, y, width, height
    150.0f,   // followSpeed: 遅め
    500.0f,   // maxDistance
    true      // isActive
    };
    

    // =====================================================
    //  縦穴の壁  (Y: 720 ～ 1380)
    // =====================================================

    stage.platforms[6] = { 950.0f,  720.0f, 50.0f, 710.0f };    // 左壁
    stage.platforms[7] = { 1200.0f,  720.0f, 50.0f, 710.0f };    // 右壁

    // =====================================================
    //  地下レイヤー  (Y: 1380 ～ 2160)
    //  天井開口部: x=700~900（縦穴に合わせる）
    // =====================================================

    stage.platforms[8] = { 0.0f,    1380.0f,  200.0f,  150.0f }; // 天井・左
    stage.platforms[9] = { 1200.0f,  1380.0f, 900.0f,  150.0f }; // 天井・右
    stage.platforms[15] = { 2250.0f, 1380.0f,  300.0f,  150.0f }; // 天井右
    stage.platforms[10] = { 0.0f,    2100.0f, 1700.0f, 200.0f }; // 地下の床
    stage.platforms[11] = { 0.0f,  1380.0f,  70.0f,  1300.0f }; //左壁
	stage.platforms[12] = { 1400.0f,  1620.0f,  150.0f,  20.0f }; 
    stage.platforms[14] = { 2000.0f, 1740.0f,  250.0f,  20.0f }; // 右の足場
    stage.platforms[13] = { 2600.0f, 2100.0f,  2000.0f,  200.0f }; // 地下の床 右
    stage.platforms[18] = { 500.0f, 1800.0f,  100.0f,  20.0f };
    stage.platforms[19] = { 0.0f, 1700.0f,  400.0f,  60.0f };
    stage.platforms[20] = { 350.0f, 1380.0f,  650.0f,  150.0f }; // 天井・左
    stage.platformCount = 64;   // 最大インデックス21 → 22個 (旧: 64 は誤り)


    //エレベーター
    stage.elevators[0] = {
    { 200.0f, 1650.0f, 150.0f, 20.0f }, // rect（初期位置＝地下の底付近）
    650.0f,    // upperY（地上付近まで上昇）
    1550.0f,   // lowerY（地下の底）
    300.0f,    // speed
    false,     // goingUp
    false,     // goingDown
    false      // onPlayer
    };
    stage.elevatorsInit[0] = stage.elevators[0];
    stage.elevatorCount = 1;

	//下から触れただけで壊れるブロック
    stage.bottomBreakBlocks.Clear();
    stage.bottomBreakBlocks.Add({ {200.0f, 1380.0f, 150.0f, 150.0f}, false, 0.3f });
    stage.bottomBreakBlocks.Add({ {200.0f, 650.0f, 150.0f, 70.0f}, false, 0.3f });
    stage.bottomBreakBlocks.Add({ {2100.0f, 1380.0f, 150.0f, 150.0f}, false, 0.7f });
    //でてくるとげ
    // 旧実装では配置データのみ設定され、moveCountが0のため有効化されていなかった。
    // 意図しない難易度変更を避けるため、移行後も未配置のままにする。
    // stage.moveHazards.Add({ {1050,2100,70,50}, false,false,50.0f,2100.0f,800.0f });

	//追尾床
    stage.cursorPlatforms[1] = {
   { 700.0f, 1900.0f, 100.0f, 20.0f },  // rect: x, y, width, height
   150.0f,   // followSpeed: 遅め
   500.0f,   // maxDistance
   true      // isActive
    };
    stage.cursorPlatformCount = 2;

	//アイテムブロック
	stage.itemBlocks.Clear();
	stage.itemBlocks.Add({ {300.0f,1900.0f,50.0f,50.0f},false,ItemType::excessJumpPotion });
	stage.itemBlocks.Add({ { 2350.0f, 1200.0f,  50.0f,  50.0f},false,ItemType::debuffPotion });
	// itemBlockCount は削除（GimmickGroup::Size()を使用）
	//壊せるブロック
	stage.breakableBlocks.Clear();
	stage.breakableBlocks.Add({ {240.0f,1900.0f,50.0f,50.0f},false });
	stage.breakableBlocks.Add({ {360.0f,1900.0f,50.0f,50.0f},false });

    // ボタンブロック（下から叩くとブロック出現）
    stage.buttonBlocks.Clear();
    stage.buttonBlocks.Add({
        {1800.0f, 400.0f, 50.0f, 50.0f},{
        {1800.0f, 2100.0f, 100.0f, 50.0f},{2000.0f, 2100.0f, 100.0f, 50.0f},{2200.0f, 2100.0f, 100.0f, 50.0f}
        },// 出現するブロック
        false
    });

	// =====================================================
	//  敵キャラ配置
    enemyManager.Spawn(EnemyType::WALKER, { 500.0f, 600.0f });
    enemyManager.enemies.back()->SetDialogKey("death_enemy_walker_st2patrol");  // 巡回中の敵

 //  分割床（地上）
  static const Rectangle SPLIT_BASES[] = { 
        { 2350.0f, 1880.0f, 200.0f, 50.0f },
    };
    stage.splitPlatformCount = (int)(sizeof(SPLIT_BASES) / sizeof(SPLIT_BASES[0]));
    for (int i = 0; i < stage.splitPlatformCount; i++) {
        auto& sp = stage.splitPlatforms[i];
        sp.base = SPLIT_BASES[i];
        sp.triggered = false;
        sp.offsetX = 0.0f;
        sp.maxGap = 60.0f;
        sp.splitSpeed = 450.0f;
        sp.left = { sp.base.x,                        sp.base.y, sp.base.width / 2.0f, sp.base.height };
        sp.right = { sp.base.x + sp.base.width / 2.0f, sp.base.y, sp.base.width / 2.0f, sp.base.height };
        sp.baseInit = SPLIT_BASES[i];
    }

 



    // =====================================================
    //  ステージサイズ（縦3画面分 720×3）
    // =====================================================

    stage.stageWidth  = 4400.0f;
    //stage.stageHeight = 2160.0f;

    // =====================================================
    //  カメラ設定（地上/地下固定、シャフトのみ追従）
    // =====================================================

    // --- 2段構造カメラ設定 ---
    // stageHeight をステージ全体の高さ（地上＋地下）に設定
    stage.stageHeight = 2160.0f;   // 地上720px + シャフト720px + 地下720px

    stage.cameraConfig.twoLayered  = true;
    // プレイヤー中心Y がこの値を超えたらシャフト追従開始（地上の床付近）
    stage.cameraConfig.shaftEnterY = 680.0f;
    // プレイヤー中心Y がこの値を超えたら地下固定（地下セクション上端付近）
    stage.cameraConfig.shaftExitY  = 1480.0f;
    stage.cameraConfig.lerpSpeed   = 8.0f;
    // ステージ3だけ、地上ゾーンでY=0より上（マイナス方向）へジャンプした際も
    // カメラが追従するようにする
    stage.cameraConfig.extendUpperFollow = true;

    // =====================================================
    //  復活用コピー
    // =====================================================
        // 復活用
    }



