#include"raylib.h"
#include "Stage.h"
#include"Item.h"
#include "ItemManager.h"
#include"StageCollision.h"

// Item.cpp の役割: アイテム単体の初期化/更新/描画処理を実装する。
void ItemInit(Item& item, ItemType type, Vector2 position) {
	item.type = type;
	item.rect = { position.x,position.y,32,32 };
	item.isActive = true;//効果中か
	item.vel = { 0.0f, 0.0f };
	item.isEmerged = false;//出現したか
	item.targetY = position.y;//目標位置
	switch (type) {
	case ItemType::jumpPotion:
		item.duration = 5.0f;
		break;
	case ItemType::speedPotion:
		item.duration = 5.0f;
		break;
	case ItemType::invinciblePotion:
		item.duration = 3.0f;
		break;
	case ItemType::excessJumpPotion:
		item.duration = 10.0f;
		break;
	}
}
// アイテムとプレイヤーの当たり判定
void ItemCollision(Item& item, const Rectangle& player, float dt, Vector2 velocity)
{
	// 非表示／未出現のアイテムは当たり判定しない
	if (!item.isActive && !item.isEmerged) return;

	if (CheckCollisionRecs(item.rect, player)) {
		// ここでは状態変更しない。ItemManager 側で効果適用してアイテムを消す想定
	}
}
// 効果中のアイテムを更新
// 目的: 出現演出中(isEmerged)の位置更新と有効化切替を管理する。
void ItemUpdate(Item& item, float dt,Vector2& velocity) {
	// 非表示／未出現のアイテムは無視
	if (!item.isActive && !item.isEmerged) return;

	// 飛び出しアニメーション処理
	if (item.isEmerged) {
		// vel.y が負なら上方向に移動するので足し算で良い
		item.rect.y += item.vel.y * dt;
		// 目標に到達したか（上方向に飛び出すことを想定）
		if (item.vel.y < 0.0f) {
			if (item.rect.y <= item.targetY) {
				item.rect.y = item.targetY;
				item.isEmerged = false;
				item.isActive = true; // 到達後に回収可能にする
				item.vel = { 0.0f, 0.0f };

			}
		}
		else { // 万が一下方向の初速が渡された場合の安全処理
			if (item.rect.y >= item.targetY) {
				item.rect.y = item.targetY;
				item.isEmerged = false;
				item.isActive = true;
				item.vel = { 0.0f, 0.0f };
			}
		}
		return;
	}
	

}

void ItemDraw(const Item& item) {
	if (!item.isActive && !item.isEmerged) return;//非表示／未出現のアイテムは描画しない
	switch (item.type) {
	case ItemType::jumpPotion:
		DrawRectangleRec(item.rect, BLUE);
		break;
	case ItemType::speedPotion:
		DrawRectangleRec(item.rect, GREEN);
		break;
	case ItemType::invinciblePotion:
		DrawRectangleRec(item.rect, YELLOW);
		break;
	case ItemType::excessJumpPotion:
		DrawRectangleRec(item.rect, PURPLE);
		break;
	}

}

void ItemReset(Item& item) {
	item.isActive = false;
	item.isEmerged = false;
	item.duration = 0.0f;
	item.vel = { 0.0f, 0.0f };
	item.targetY = 0.0f;
}