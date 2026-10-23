#pragma once
#include "raylib.h"
#include <vector>
#include <string>

// タイルセット1つ分のデータ
struct TilesetData {
    Texture2D texture{};
    int firstGid = 1;
    int tileW = 16;
    int tileH = 16;
    int columns = 1;
};

// タイルレイヤー1枚分のデータ
struct TileLayerData {
    std::string    name;
    std::vector<int> gids;   // 0 = 空タイル
    int   width = 0;
    int   height = 0;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    bool  visible = true;
};

// Tiled マップの描画用データ一式
struct TiledMap {
    std::vector<TilesetData>  tilesets;
    std::vector<TileLayerData> tileLayers;
    int  mapWidth = 0;
    int  mapHeight = 0;
    int  tileW = 16;
    int  tileH = 16;
    bool loaded = false;

    void Draw()   const;
    void Unload();
};