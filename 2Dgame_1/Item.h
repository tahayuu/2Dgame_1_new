#pragma once
#include "raylib.h"
#include <string>

enum class ItemType {
    jumpPotion,
    speedPotion,
    invinciblePotion,
    excessJumpPotion,
    debuffPotion
};

struct Item {
    ItemType type;
    Rectangle rect;
    bool isActive;//効果中か
    float duration = 5.0f;//効果時間

    //ブロックから出現用
    Vector2 vel = { 0.0f,0.0f };//速度
    bool isEmerged = false;//出現したか
    float targetY = 50.0f;
};


void ItemInit(Item& item, ItemType type, Vector2 position);
void ItemCollision(Item& item, const Rectangle& player, float dt, Vector2 velocity);
void ItemUpdate(Item& item, float dt,Vector2& velocity);

void ItemDraw(const Item& item);
void ItemReset(Item& item);