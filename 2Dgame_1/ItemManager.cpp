#include "ItemManager.h"

// ================================================================
// ItemManager.cpp の役割
// ---------------------------------------------------------------
// ・アイテム本体の更新と、回収後効果(activeItems)の時間管理を実装する。
// ・Playerは GetSpeedBoost/GetJumpBoost を通して効果値を受け取る。
// ================================================================

void ItemManager::Init()
{
	//•	ゲーム開始・ステージ切替時に内部配列を「空」にして、追加に備えるため。
	items.clear();//既存のデータをクリア
	items.reserve(32); //あらかじめ32体分のメモリを確保);
	activeItems.clear();
	invincible = false;

}


void ItemManager::Spawn(ItemType Type, Vector2 pos,Vector2 initialVel, bool emerging) {//ステージ上にアイテムを出現させる（プレイヤーが近づいたときなどに呼ぶ想定）
	Item item;
	ItemInit(item, Type, pos);
	if (emerging) {
		item.isEmerged = true;
		item.vel = initialVel;
		item.targetY = pos.y - 80.0f;
		item.isActive = false;
	}
	else {
		item.isActive = true;
	}
	
	items.push_back(item);

}          


// 敵とプレイヤーの当たり判定
void ItemManager::ItemCollisionAll(Item& item, const Rectangle& player, float dt, Vector2& velocity) {
	for (auto& item : items) {
		// 当たり判定は回収可能なアイテムにのみ適用
		if ((item.isActive) && CheckCollisionRecs(item.rect, player)) {
			
			ApplyItemEffect(item.type, *this, item); 
	
			item.isActive = false;
		}
	}
} 

// 目的: ワールドアイテム状態と、効果中アイテムの残り時間を同時更新する。
// 注意: activeItems の管理は Playerの移動性能に直結するため削除条件変更に注意。
void ItemManager::UpdateAll(float dt, const Rectangle& player,Vector2& velocity, ItemBlock* itemBlock) {
	for (auto& item : items) {
		// 飛び出し中も更新して移動させる
		if (item.isActive || item.isEmerged) ItemUpdate(item, dt, velocity);//効果中でなければ更新
	}

	// 回収済み／完全に非表示のアイテムのみ削除（出現中のアイテムは残す）
	items.erase(std::remove_if(items.begin(), items.end(),
		[](const Item& item) {return !item.isActive && !item.isEmerged; }),
		items.end());
	if (!activeItems.empty()) {//効果が適用中  
		for (auto& activeItem : activeItems) {
			activeItem.remainingDuration -= dt;
		}
		activeItems.erase(
			std::remove_if(activeItems.begin(), activeItems.end(),
				[](const ActiveItem& activeItem) { return activeItem.remainingDuration <= 0.0f; }),
			activeItems.end());
	}
	
	invincible = false;
	for (const auto& a : activeItems) {
		if (a.type == ItemType::invinciblePotion) {
			invincible = true;
			break;
		}
	}
} 

// すべて描画
void ItemManager::DrawAll() { 
	for (auto& item : items) {
		if (!item.isActive && !item.isEmerged) continue;

        const Texture2D* tex = nullptr;
        switch (item.type) {
        case ItemType::speedPotion:      tex = &texSpeedPotion;      break;
        case ItemType::jumpPotion:       tex = &texJumpPotion;       break;
        case ItemType::excessJumpPotion: tex = &texExcessJumpPotion; break;
        case ItemType::invinciblePotion: tex = &texInvinciblePotion; break;
        case ItemType::debuffPotion:     tex = &texDebuffPotion;     break;
        }

        if (tex != nullptr && tex->id != 0) {
            Rectangle src = { 0, 0, (float)tex->width, (float)tex->height };
            DrawTexturePro(*tex, src, item.rect, { 0, 0 }, 0, WHITE);
        }
        else {
            ItemDraw(item);
        }
	}
}                           

// すべて無効化 / リセット
void ItemManager::Reset() {    
	for (auto& item : items) {
		ItemReset(item);
	}
	items.clear();
	activeItems.clear();
	invincible = false;
}                            

// 目的: 回収されたアイテムを activeItems へ登録し、必要なら即時フラグを更新する。
void ItemManager::ApplyItemEffect(ItemType type, ItemManager& itemManag, Item& item) {
	float duration = 5.0f;
	if (type == ItemType::invinciblePotion)duration = itemManag.invincibleDuration;
	else duration = item.duration;

	switch (type) {
	case ItemType::speedPotion: {
		itemManag.activeItems.push_back({ type,duration });//末尾に{ type,duration }を追加
		item.isActive = false;
		break;
	}
     
	case ItemType::jumpPotion: {
		itemManag.activeItems.push_back({ type,duration });
		item.isActive = false;
		break;
	}

	case ItemType::invinciblePotion: {
		itemManag.activeItems.push_back({ type, duration });
		itemManag.invincible = true;
		item.isActive = false;
		break;
	}
	case ItemType::excessJumpPotion: {
		itemManag.activeItems.push_back({ type,duration });//末尾に{ type,duration }を追加
		item.isActive = false;
		break;
	}
	case ItemType::debuffPotion: {
		itemManag.activeItems.push_back({ type, duration });
		item.isActive = false;
		break;
	}
	}

}

void ItemManager::saveItemsInit() {
	itemsInit = items;
}

void ItemManager::RestorInitialItems() {
	items = itemsInit;
	for (auto& item : items) {
		item.isActive = true;//全て有効化
		item.isEmerged = false;
		item.vel = {0.0f,0.0f};
	}
	activeItems.clear();
	invincible = false;

}

float ItemManager::GetJumpBoost() const {
	float total = 0.0f;
	bool hasExcess = false;
	bool hasDebuff = false;

	for (const auto& activeItem : activeItems) {
		if (activeItem.type == ItemType::jumpPotion) {
			total += jumpBoostAmount;
		}
		else if (activeItem.type == ItemType::excessJumpPotion) {
			hasExcess = true;
		}
		else if (activeItem.type == ItemType::debuffPotion) {
			hasDebuff = true;
		}
	}

	if (hasExcess && hasDebuff) {
		// 同時取得：excessJumpBoostAmount - debuffJumpReduction = 通常より少し高い
		total += excessJumpBoostAmount - debuffJumpReduction;
	}
	else if (hasExcess) {
		total += excessJumpBoostAmount;
	}
	else if (hasDebuff) {
		total -= debuffJumpReduction;
	}

	return total;
}

float ItemManager::GetSpeedBoost() const {
	float total = 0.0f;
	for (const auto& activeItem : activeItems) {
		if (activeItem.type == ItemType::speedPotion) {
			total += speedBoostAmount;
		}
		else if (activeItem.type == ItemType::debuffPotion) {
			total -= debuffSpeedReduction; // 速度は常に減少（excess同時でも）
		}
	}
	return total;
}


float ItemManager::GetActiveItems() const {
	return GetSpeedBoost() + GetJumpBoost();
}

void ItemManager::Load() {
    texSpeedPotion      = LoadTexture("assets/images/items/speedportion.png");
    texJumpPotion       = LoadTexture("assets/images/items/jumppotion.png");
    texExcessJumpPotion = LoadTexture("assets/images/items/excessjumppotion.png");
    texInvinciblePotion = LoadTexture("assets/images/items/invincible_potion.png");
	texDebuffPotion = LoadTexture("assets/images/items/debuffpotion.png");
}

void ItemManager::Unload() {
    if (texSpeedPotion.id      != 0) UnloadTexture(texSpeedPotion);
    if (texJumpPotion.id       != 0) UnloadTexture(texJumpPotion);
    if (texExcessJumpPotion.id != 0) UnloadTexture(texExcessJumpPotion);
    if (texInvinciblePotion.id != 0) UnloadTexture(texInvinciblePotion);
	if (texDebuffPotion.id != 0) UnloadTexture(texDebuffPotion);
    texSpeedPotion      = Texture2D{};
    texJumpPotion       = Texture2D{};
    texExcessJumpPotion = Texture2D{};
    texInvinciblePotion = Texture2D{};
	texDebuffPotion = Texture2D{};
}