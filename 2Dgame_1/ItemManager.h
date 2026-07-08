#pragma once
#include"raylib.h"
#include "Item.h"
#include <vector>
#include "GameObjects.h" 

// ================================================================
// ItemManager.h の役割
// ---------------------------------------------------------------
// ・ワールド上アイテムの出現/回収/描画を管理する。
// ・回収後の効果(ActiveItem)を時間管理し、Playerへブースト値を提供する。
// ================================================================
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

	// 目的: 回収時にアイテム種別ごとの効果を activeItems へ反映する。
	void ApplyItemEffect(ItemType type, ItemManager& itemManag, Item& item);

	// 目的: 管理配列と効果状態を初期化する。
	void Init();
	// 目的: アイテム1つを生成して管理配列へ追加する。
	void Spawn(ItemType Type, Vector2 pos, Vector2 initialVel = {0.0f,0.0f}, bool emerging = false);

	// 目的: プレイヤー回収判定を行う。
	void ItemCollisionAll(Item& item, const Rectangle& player, float dt, Vector2& velocity);
	// 目的: アイテム移動・効果時間・無敵状態を更新する。
	void UpdateAll(float dt, const Rectangle& player,Vector2& velocity, ItemBlock* itemBlock = nullptr);
	// 目的: ワールド上の有効アイテムを描画する。
	void DrawAll();
	// 目的: 管理状態をリセットする。
	void Reset();
	// 目的: 現在配置を初期配置として保存する。
	void saveItemsInit();

	// 目的: 保存済み初期配置を復元する。
	void RestorInitialItems();

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