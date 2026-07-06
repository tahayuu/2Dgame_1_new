#include "SpriteDatabase.h"
#include <cstring>

namespace {

    // ================================================================
    // スプライト定義テーブル
    // ----------------------------------------------------------------
    // アトラス方式とは違い、1つの SpriteId につき「切り出し済みの1枚のPNG」を
    // まるごと読み込む方式にしている。
    //
    // path : assets/images/sprites/ 以下に置く画像ファイルのパス。
    //        ★まだ画像を用意していない場合は空文字列のままでOK
    //          （その場合は読み込みをスキップし、DrawSprite側で何もしない）。
    //
    // 新しい画像パーツを増やしたいときは、
    //   1. SpriteDatabase.h の SpriteId に列挙子を追加する（COUNT の直前）
    //   2. ここに対応する行（name, path）を追加する
    // の2箇所を変更すればOK。
    // ================================================================
    struct SpriteEntry {
        SpriteId id;
        const char* name;
        const char* path;
    };

    const SpriteEntry g_spriteTable[] = {
        // id                      name             path（切り出し済みPNG）
        { SpriteId::None,          "None",          "" }, // 見た目なし
        { SpriteId::MetalFloor01,  "MetalFloor01",  "assets/images/stage/stage_3/tile1.png" },
        { SpriteId::MetalFloor02,  "MetalFloor02",  "assets/images/stage/stage_3/tile2.png" },
        { SpriteId::MetalWall01,   "MetalWall01",   "assets/images/stage/stage_3/tile3.png" },
        { SpriteId::MetalWall02,   "MetalWall02",   "assets/images/stage/stage_3/object1.png" },
        { SpriteId::Ladder01,      "Ladder01",      "assets/images/stage/stage_3/object2.png" },
        { SpriteId::PipeStraight,  "PipeStraight",  "assets/images/stage/stage_3/floar2.png" },
        { SpriteId::PipeCorner,    "PipeCorner",    "assets/images/stage/stage_3/wall1.png" },
        { SpriteId::Vent,          "Vent",          "assets/images/stage/stage_3/vent.png" },
        { SpriteId::Door,          "Door",          "assets/images/stage/stage_3/door.png" },
        { SpriteId::Fence,         "Fence",         "assets/images/stage/stage_3/fence.png" },
        { SpriteId::Beam,          "Beam",          "assets/images/stage/stage_3/beam.png" },
    };

    constexpr int kSpriteTableCount = sizeof(g_spriteTable) / sizeof(g_spriteTable[0]);

    // SpriteId ごとに読み込んだテクスチャを保持する配列
    // （g_spriteTable と同じ並び順・同じ要素数で対応させる）
    Texture2D g_textures[kSpriteTableCount] = {};

    // g_spriteTable / g_textures 内でのインデックスを SpriteId から検索する
    int FindEntryIndex(SpriteId id) {
        for (int i = 0; i < kSpriteTableCount; i++) {
            if (g_spriteTable[i].id == id) return i;
        }
        return -1;
    }

} // namespace（無名）

// ----------------------------------------------------------------
// ゲーム起動時に1回だけ呼ぶ想定。
// ----------------------------------------------------------------
void SpriteDatabase::Load() {
    for (int i = 0; i < kSpriteTableCount; i++) {
        // 既に読み込み済みなら二重読み込みしない
        if (g_textures[i].id != 0) continue;

        const char* path = g_spriteTable[i].path;
        if (path == nullptr || path[0] == '\0') continue; // 未設定はスキップ

        g_textures[i] = LoadTexture(path);

        // 読み込みに失敗しても落ちないようにしてある（その場合 id==0 のまま）。
        // DrawSprite 側で id==0 のチェックをしているので、
        // 画像がまだ無い環境でも今まで通りの仮描画にフォールバックできる。
    }
}

void SpriteDatabase::Unload() {
    for (int i = 0; i < kSpriteTableCount; i++) {
        if (g_textures[i].id != 0) {
            UnloadTexture(g_textures[i]);
        }
        g_textures[i] = Texture2D{};
    }
}

Texture2D SpriteDatabase::GetTexture(SpriteId id) {
    int idx = FindEntryIndex(id);
    if (idx < 0) return Texture2D{};
    return g_textures[idx];
}

const char* SpriteDatabase::GetSpriteName(SpriteId id) {
    int idx = FindEntryIndex(id);
    if (idx < 0) return "Unknown";
    return g_spriteTable[idx].name;
}

SpriteId SpriteDatabase::FindSpriteIdByName(const char* name) {
    if (name == nullptr) return SpriteId::None;
    for (int i = 0; i < kSpriteTableCount; i++) {
        if (std::strcmp(g_spriteTable[i].name, name) == 0) return g_spriteTable[i].id;
    }
    return SpriteId::None;
}

void SpriteDatabase::DrawSprite(SpriteId id, Rectangle dst, float rotation, bool flipX, bool flipY, Color tint) {
    // 見た目が設定されていない場合は何もしない
    // （呼び出し側で「今まで通りの仮描画」をしてもらう想定）
    if (id == SpriteId::None) return;

    Texture2D tex = GetTexture(id);
    // そのSpriteId用のテクスチャがまだ読み込まれていない場合も何もしない
    if (tex.id == 0) return;

    // 切り出し済みPNGなので、切り抜きは行わず画像全体をそのまま使う
    Rectangle src = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };

    // 反転は「切り抜き範囲の幅・高さの符号を反転させる」ことで実現する
    // （raylibのDrawTexturePro定番テクニック）
    if (flipX) src.width = -src.width;
    if (flipY) src.height = -src.height;

    // 回転の軸を dst の中心にするため、
    // 「中心座標」＋「originオフセット」の形でDrawTextureProに渡す
    Vector2 origin = { dst.width * 0.5f, dst.height * 0.5f };
    Rectangle dstCentered = {
        dst.x + dst.width * 0.5f,
        dst.y + dst.height * 0.5f,
        dst.width,
        dst.height
    };

    DrawTexturePro(tex, src, dstCentered, origin, rotation, tint);
}