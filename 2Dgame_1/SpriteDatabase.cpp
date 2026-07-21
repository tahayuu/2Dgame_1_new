#include "SpriteDatabase.h"
#include <cstring>

namespace {

    // ================================================================
    // スプライト定義テーブル
    // ----------------------------------------------------------------
    // アトラス方式とは違い、1つの SpriteId につき「切り出し済みの1枚のPNG」を
    // まるごと読み込む方式にしている。
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
        /*stage_3*/
        // id                      name             path（切り出し済みPNG）
        { SpriteId::None,          "None",          "" }, // 見た目なし   1
        {SpriteId::Douka,          "Douka",    "assets/images/stage/stage_1/douka.png"},//2
        { SpriteId::MetalFloor01,  "MetalFloor01",  "assets/images/stage/stage_3/wall_2.png" },//3
        { SpriteId::MetalFloor02,  "MetalFwoor02",  "assets/images/stage/stage_3/wall_1.png" },//4
        { SpriteId::MetalWall01,   "MetalWall01",   "assets/images/stage/stage_3/ground_1.png" },//5
		{ SpriteId::MetalWall02,   "MetalWall02",   "assets/images/stage/stage_3/object1.png" },//6
		{ SpriteId::Ladder01,      "Ladder01",      "assets/images/stage/stage_3/object2.png" },//7
		{ SpriteId::PipeStraight,  "PipeStraight",  "assets/images/stage/stage_3/floar2.png" },//8
		{ SpriteId::PipeCorner,    "PipeCorner",    "assets/images/stage/stage_3/wall1.png" },//9
		{ SpriteId::Vent,          "Vent",          "assets/images/stage/stage_3/nedle1.png" },//10
		{ SpriteId::Door,          "Door",          "assets/images/stage/stage_3/nedle2.png" },//11
		{ SpriteId::Nedle3,          "Nedle3",          "assets/images/stage/stage_3/nedle3.png" },//12
		{ SpriteId::CursorPlatform, "CursorPlatform", "assets/images/stage/stage_3/CursorBlock.png" },//13
		{SpriteId::Dassh,          "Dassh",    "assets/images/stage/stage_3/dassh_1.png"},//14
		{SpriteId::Arrow,          "Arrow",    "assets/images/stage/stage_3/Arrow_1.png"},//15
        {SpriteId::Buttom_break,          "Buttom_break",    "assets/images/stage/stage_3/Buttom_break.png"},//16
        {SpriteId::Exitdoor_2,          "Exitdoor_2",    "assets/images/stage/stage_3/Exitdoor_2.png"},//17
        {SpriteId::Exitdoor2_fake,          "Exitdoor2_fake",    "assets/images/stage/stage_3/Exitdoor_fake.png"},//18

        {SpriteId::tempBlock,          "tempBlock",    "assets/images/stage/stage_3/tempBlock.png"},//19
        {SpriteId::Drum_1,          "Drum_1",    "assets/images/stage/stage_3/drum_1.png"},//20
		{SpriteId::Drum_2,          "Drum_2",    "assets/images/stage/stage_3/drum_2.png"},//21



        /*stage_1 stage_2*/
        { SpriteId::Fence,         "Fence",         "assets/images/stage/stage_1/signbord1.png" },//22
        { SpriteId::Beam,          "Beam",          "assets/images/stage/stage_1/ground.png" },//23
        { SpriteId::ground1,         "ground1",         "assets/images/stage/stage_1/ground_2.png" },//24
        { SpriteId::ground2,          "ground2",          "assets/images/stage/stage_1/ground.png" },//25
        { SpriteId::ground2,          "ground2",          "assets/images/stage/stage_1/ground.png" },//26
        {SpriteId::Exitdoor_1,          "Exitdoor_1",    "assets/images/stage/stage_1/Exitdoor_1.png"},//27
        {SpriteId::Enemy_auto,          "enemy_auto",    "assets/images/stage/stage_1/enemy_auto.png"},//28
        {SpriteId::NextStage_1,          "NextStage_1",    "assets/images/stage/stage_1/NextStage.png"},//29
        {SpriteId::Bullet,          "Bullet",    "assets/images/stage/stage_1/Bullet.png"},//30
		//ステージ３
        {SpriteId::NextStage_2,          "NextStage_2",    "assets/images/stage/stage_3/nextStage.png"},//31    
		 {SpriteId::NextStage_fake,          "NextStage_fake",    "assets/images/stage/stage_3/necstStage.png"},//32
		{SpriteId::Stage_2,          "Stage_2",    "assets/images/stage/stage_1/Stage_2.png"},//33
		{SpriteId::Stage_3,          "Stage_3",    "assets/images/stage/stage_3/Stage_3.png"},//34
        //ステージ４
		{SpriteId::Stage_4,          "Stage_4",    "assets/images/stage/stage_4/Stage_4.png"},//35
        {SpriteId::DIE_block,          "DIE_block",    "assets/images/stage/stage_4/DIE_block.png"},//36
        {SpriteId::Fake_clear,          "Fake_clear",    "assets/images/stage/stage_4/Fake_clear.png"},//37
         {SpriteId::IceBlock,          "IceBlock",    "assets/images/stage/stage_4/iceBlock.png"},//38
         {SpriteId::Jump_Space,          "Jump_Space",    "assets/images/stage/stage_4/jump_Space.png"},//39
         {SpriteId::Jump_N,          "Jump_N",    "assets/images/stage/stage_4/jump_N.png"},//40
         {SpriteId::PuzzleBridge,          "PuzzleBridge",    "assets/images/stage/stage_4/puzzleBridge.png"},//41
		 {SpriteId::PuzzleEdg,          "PuzzleEdg",    "assets/images/stage/stage_4/puzzleEdg.png"},//42
         {SpriteId::bored_brigename,          "bored_brigename",    "assets/images/stage/stage_4/bored_brigename.png"},//43
         {SpriteId::bored_wataru,          "bored_wataru",    "assets/images/stage/stage_4/bored_wataru.png"},//44
          {SpriteId::brige_road,          "brige_road",    "assets/images/stage/stage_4/brige_road.png"},//45
         {SpriteId::brige_saku,          "brige_saku",    "assets/images/stage/stage_4/brige_saku.png"},//46
         {SpriteId::brige_saku2,          "brige_saku2",    "assets/images/stage/stage_4/brige_saku2.png"},//47
		 //おじさん
         {SpriteId::Ojisan_car,          "Ojisan_car",    "assets/images/ojisan/ojisan_car.png"},//48
         {SpriteId::Ojisan_real1,          "Ojisan_real1",    "assets/images/ojisan/ojisan_real1.png"},//49
         {SpriteId::Ojisan_real2,          "Ojisan_real2",    "assets/images/ojisan/ojisan_real2.png"},//50
         {SpriteId::Ojisan_real3,          "Ojisan_real3",    "assets/images/ojisan/ojisan_real3.png"},//51
         {SpriteId::Ojisan_kick,          "Ojisan_kick",    "assets/images/ojisan/ojisan_kick.png"},//52    
         {SpriteId::Ojisan_waruguti1,          "Ojisan_waruguti1",    "assets/images/ojisan/ojisan_waruguti1.png"},//53
         {SpriteId::Ojisan_waruguti2,          "Ojisan_waruguti2",    "assets/images/ojisan/ojisan_waruguti2.png"},//54
         {SpriteId::Ojisan_waruguti3,          "Ojisan_waruguti3",    "assets/images/ojisan/ojisan_waruguti3.png"},//55             
        {SpriteId::Ojisan_idle,          "Ojisan_idle",    "assets/images/ojisan/ojisan_idle2.png"},//56
        {SpriteId::Ojisan_notWalk,          "Ojisan_notWalk",    "assets/images/ojisan/ojisan_notWalk.png"},//57
        {SpriteId::canWalk,          "canWalk",    "assets/images/stage/stage_4/canWalk.png"},//58
       {SpriteId::notWalk,          "notWalk",    "assets/images/stage/stage_4/notWalk.png"},//59
              {SpriteId::enemy_muteki,          "enemy_muteki",    "assets/images/enemy/enemy_muteki.png"},//60
              {SpriteId::enemy_teki,          "enemy_teki",    "assets/images/enemy/enemy_teki.png"},//61
              {SpriteId::mu,          "mu",    "assets/images/enemy/mu.png"},//62
               {SpriteId::Ojisan_real5,          "Ojisan_real5","assets/images/ojisan/ojisan_real5.png"},//63
                {SpriteId::give,          "give",    "assets/images/stage/stage_4/give.png"},//64
                {SpriteId::punch_4,          "punch_4",    "assets/images/stage/stage_4/punch_4.png"},//65
                {SpriteId::punch_comment,          "punch_comment",    "assets/images/stage/stage_4/punch_comment.png"},//66
				{SpriteId::punch_comment_2,          "punch_comment_2",    "assets/images/stage/stage_4/punch_comment_2.png"},//67
				{SpriteId::punch_comment_3,          "punch_comment_3",    "assets/images/stage/stage_4/punch_comment_3.png"},//68
				{SpriteId::punch_comment_4,          "punch_comment_4",    "assets/images/stage/stage_4/punch_comment_4.png"},//69
                {SpriteId::fuck,          "fuck",    "assets/images/stage/stage_4/fuck_2.png"},//70
                {SpriteId::move,          "move",    "assets/images/stage/stage_4/move.png"},//71
                {SpriteId::exitdoor4,          "exitdoor4",    "assets/images/stage/stage_4/exitdoor.png"},//72
    };
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