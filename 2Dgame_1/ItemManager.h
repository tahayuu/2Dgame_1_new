#pragma once
#include"raylib.h"
#include "Item.h"
#include <vector>
#include "GameObjects.h" 
//•	ItemManager はゲーム内のアイテム（床にある回収可能なオブジェクト）をまとめて管理
// ワールド上のアイテム（出現・描画・当たり判定）の管理と、プレイヤーに付与された効果（activeItems）
// の経過管理
struct ItemManager {
	std::vector<Item> items;//可変長配列
	std::vector<Item> itemsInit;//初期配置用配列
	float jumpBoostAmount = 200.0f; // ジャンプ力アップ量
	float excessJumpBoostAmount = 1300.0f; //過剰なジャンプ力
	float speedBoostAmount = 200.0f; // 速度アップ量
	float invincibleDuration = 3.0f; // 無敵効果時間
	float debuffJumpReduction = 1150.0f;
	float debuffSpeedReduction = 300.0f;
	bool invincible = false;

    struct ActiveItem {
        ItemType type;
        float remainingDuration;
	};
	std::vector<ActiveItem> activeItems; // 現在効果中のアイテムリスト

	void ApplyItemEffect(ItemType type, ItemManager& itemManag, Item& item); // アイテム効果を適用

    void Init();                                         // 初期化（必要ならプリロード）
	// Spawn の引数にデフォルト値を付与（呼び出し側の互換性確保)
	void Spawn(ItemType Type, Vector2 pos, Vector2 initialVel = {0.0f,0.0f}, bool emerging = false);        // アイテムを生成（追加）
	
    void ItemCollisionAll(Item& item, const Rectangle& player, float dt, Vector2& velocity); // 敵とプレイヤーの当たり判定
    void UpdateAll(float dt, const Rectangle& player,Vector2& velocity, ItemBlock* itemBlock = nullptr);   // すべて更新
    void DrawAll();                                      // すべて描画
    void Reset();            // すべて無効化 / リセット
	void saveItemsInit(); //初期配置保存

	void RestorInitialItems(); //初期配置に戻す

	float GetActiveItems() const; // 効果中のアイテム情報を取得
	float GetSpeedBoost() const;
	float GetJumpBoost() const;

	// アイテムテクスチャ
	Texture2D texSpeedPotion{};
	Texture2D texJumpPotion{};
	Texture2D texExcessJumpPotion{};
	Texture2D texInvinciblePotion{};
	Texture2D texDebuffPotion{};

	void Load();
	void Unload();
};