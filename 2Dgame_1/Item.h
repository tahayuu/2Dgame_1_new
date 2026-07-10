#pragma once
#include "raylib.h"
#include <string>

// ================================================================
// Item.h の役割
// ---------------------------------------------------------------
// ・アイテム1個分の状態と、単体更新APIを定義する。
// ・効果適用そのものは ItemManager 側で行う分担にしている。
// ================================================================

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


// 目的: アイテム初期値を種別ごとに設定する。
void ItemInit(Item& item, ItemType type, Vector2 position);
// 目的: プレイヤーとの接触判定を行う（効果適用はManager側）。
void ItemCollision(Item& item, const Rectangle& player, float dt, Vector2 velocity);
// 目的: 飛び出し中アイテムなどの位置状態を更新する。
void ItemUpdate(Item& item, float dt,Vector2& velocity);

// 目的: 単体アイテムを描画する（テクスチャ未使用時フォールバック）。
void ItemDraw(const Item& item);
// 目的: アイテム状態を無効化方向へリセットする。
void ItemReset(Item& item);
