#pragma once
#include "Enemy.h"
#include <vector>

// ================================================================
// EnemyManager.h の役割
// ---------------------------------------------------------------
// ・敵配列の生成/更新/描画/リセットを一括管理する。
// ・プレイヤー接触結果（側面衝突かどうか等）を集約して外部へ渡す。
// ================================================================

struct EnemyManager {
    std::vector<Enemy> enemies;
	std::vector<Enemy> enemiesInit; //初期配置用配列
    //可変長配列
    //サイズ変更可能
	//C++らしいコードを書くならこっちを使う
	//enemies.push_back(追加したい値);末尾に追加
    /*
	* for (int i{0}; i < enemies.size(); i++) {動的にサイズを取得
	* for(auto& enemy : enemies) {範囲ベースfor文
	* enemis.at(i);範囲外アクセス時に例外を投げる
    */

	bool playerTouched = false; //プレイヤーに触れたか
	bool touchedEnemyFromSide = false; //プレイヤーが触れた敵の側面からか
	int touchedEnemyIndex = -1;        //プレイヤーが触れた敵のインデックス
	std::string touchedEnemyDialogKey = ""; //プレイヤーが触れた敵のダイアログキー


    // 目的: 管理配列と接触フラグを初期化する。
        void Init();
    // 目的: 敵1体を生成して管理配列へ追加する。
        void Spawn(EnemyType Type, Vector2 pos);
	// 目的: 全敵との接触を更新し、プレイヤー接触情報を集約する。
		void EnemyCollisionAll( const Rectangle& player, float dt,Vector2& velocity);
    // 目的: 全敵のAI更新と非アクティブ削除を行う。
        void UpdateAll(float dt, const Rectangle& player);
    // 目的: 管理中の全敵を描画する。
        void DrawAll();
    // 目的: 敵状態をリセットし、配列をクリアする。
        void Reset();

	// 目的: 現在敵配置を初期状態として保存する。
		void saveEnemiesInit();
	// 目的: 保存した初期配置へ戻す。
		void RestorInitialEnemies();
};