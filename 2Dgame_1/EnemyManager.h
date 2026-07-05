#pragma once
#include "Enemy.h"
#include <vector>

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


    void Init();                                         // 初期化（必要ならプリロード）
    void Spawn(EnemyType Type, Vector2 pos);             // 敵を生成（追加）
	void EnemyCollisionAll( const Rectangle& player, float dt,Vector2& velocity); // 敵とプレイヤーの当たり判定
    void UpdateAll(float dt, const Rectangle& player);   // すべて更新
    void DrawAll();                                      // すべて描画
    void Reset();                                        // すべて無効化 / リセット

	void saveEnemiesInit(); //初期配置保存
	void RestorInitialEnemies(); //初期配置に戻す
};