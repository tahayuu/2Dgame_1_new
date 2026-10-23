#include "StageTheme.h"
#include <cmath>

// ================================================================
// StageTheme.cpp の役割
// ---------------------------------------------------------------
// ・ステージ床/ブロックの見た目リソースを読み込み・解放・描画する。
// ・StageCollision とは分離し、見た目変更が当たり判定へ波及しない構成にしている。
// ================================================================

// 目的: シンプルな上面+内部テクスチャ構成を有効化する。
void StageThemeLoad(StageTheme& theme, const char* topPath, const char* bodyPath, float tileSize) {
    theme.platformTop  = LoadTexture(topPath);
    theme.platformBody = LoadTexture(bodyPath);
    theme.tileSize     = tileSize;
    theme.useLeftRight = false;
    theme.hasTextures  = (theme.platformTop.id != 0 && theme.platformBody.id != 0);
}

void StageThemeLoadObjectTextures(
    StageTheme& theme,
    const char* itemBlockPath,
    const char* normalBlockPath,
    const char* arrowPath,
	const char* bulletPath,
    const char* ActionButtn_offPath,
	const char* ActionButtn_onPath,
	const char* touchBlockPath,
	const char* magnetPath,
	const char* magnetEffectPath,
	const char* tempBlockPath,
	const char* circlePath
) {
	theme.itemBlock = LoadTexture(itemBlockPath);
	theme.touchBlock = LoadTexture(touchBlockPath);
	theme.arrowTex = LoadTexture(arrowPath);
	theme.bulletTex = LoadTexture(bulletPath);
	theme.actionButtonOff = LoadTexture(ActionButtn_offPath);
	theme.actionButtonOn = LoadTexture(ActionButtn_onPath);
	theme.normalBlock = LoadTexture(normalBlockPath);
	theme.magnetTex = LoadTexture(magnetPath);
	theme.magnetEffectTex = LoadTexture(magnetEffectPath);
	theme.tempBlockTex = LoadTexture(tempBlockPath);
	theme.circleTex = LoadTexture(circlePath);
}

void StageThemeLoadLR(StageTheme& theme, const char* leftPath, const char* rightPath, float tileSize) {
    theme.platformLeft  = LoadTexture(leftPath);
    theme.platformRight = LoadTexture(rightPath);
    theme.tileSize      = tileSize;
    theme.useLeftRight  = true;
    theme.hasTextures   = (theme.platformLeft.id != 0 && theme.platformRight.id != 0);
}

// 目的: 4方向分の画像をまとめて読み込み、テーマを一度で完成させる。
void StageThemeLoadAll(StageTheme& theme,
    const char* topPath, const char* bodyPath,
    const char* leftPath, const char* rightPath, float tileSize)
{
    theme.platformTop   = LoadTexture(topPath);
    theme.platformBody  = LoadTexture(bodyPath);
    theme.platformLeft  = LoadTexture(leftPath);
    theme.platformRight = LoadTexture(rightPath);
    theme.tileSize      = tileSize;
    theme.useLeftRight  = true;
    theme.hasTextures   = (theme.platformTop.id != 0 && theme.platformBody.id != 0 &&
                           theme.platformLeft.id != 0 && theme.platformRight.id != 0);
}

// 目的: テーマで保持するGPUテクスチャを漏れなく解放する。
void StageThemeUnload(StageTheme& theme) {
    if (theme.platformTop.id   != 0) UnloadTexture(theme.platformTop);
    if (theme.platformBody.id  != 0) UnloadTexture(theme.platformBody);
    if (theme.platformLeft.id  != 0) UnloadTexture(theme.platformLeft);
    if (theme.platformRight.id != 0) UnloadTexture(theme.platformRight);
    if (theme.itemBlock.id != 0) UnloadTexture(theme.itemBlock);
	if (theme.normalBlock.id != 0) UnloadTexture(theme.normalBlock);
	if (theme.arrowTex.id != 0) UnloadTexture(theme.arrowTex);
	if (theme.bulletTex.id != 0) UnloadTexture(theme.bulletTex);
	if (theme.actionButtonOff.id != 0) UnloadTexture(theme.actionButtonOff);
	if (theme.actionButtonOn.id != 0) UnloadTexture(theme.actionButtonOn);
	if (theme.magnetTex.id != 0) UnloadTexture(theme.magnetTex);
	if (theme.magnetEffectTex.id != 0) UnloadTexture(theme.magnetEffectTex);
	theme = StageTheme{};
}

static void DrawTextureTiledColumn(
    const Texture2D& tex,
    float drawX, float rectY, float rectBottom,
    float tileSize, float drawW)
{
    int rows   = (int)ceilf((rectBottom - rectY) / tileSize);
    float texW = (float)tex.width;
    float texH = (float)tex.height;

    for (int r = 0; r < rows; r++) {
        float drawY = rectY + r * tileSize;
        float drawH = tileSize;
        float srcW  = texW * (drawW / tileSize);
        float srcH  = texH;

        if (drawY + drawH > rectBottom) {
            float over = (drawY + drawH) - rectBottom;
            srcH *= (drawH - over) / drawH;
            drawH -= over;
        }

        Rectangle src = { 0.0f, 0.0f, srcW, srcH };
        Rectangle dst = { drawX, drawY, drawW, drawH };
        DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
    }
}

// 目的: 床1つを tileSize 単位で分割し、端や中央を崩さずに描画する。
// 注意: ここは描画専用。衝突判定サイズは rect 側で管理する。
void DrawPlatformTextured(Rectangle rect, const StageTheme& theme) {
    if (!theme.hasTextures) {
        DrawRectangleRec(rect, DARKGRAY);
        return;
    }

    float tile = theme.tileSize;

    if (theme.useLeftRight) {
        float edgeW      = (rect.width >= tile * 2.0f) ? tile : rect.width / 2.0f;
        float midW       = rect.width - edgeW * 2.0f;
        float rectBottom = rect.y + rect.height;
        bool hasMidTex   = (theme.platformTop.id != 0 && theme.platformBody.id != 0);

        if (midW > 0.0f) {
            if (hasMidTex) {
                int midCols   = (int)ceilf(midW / tile);
                int totalRows = (int)ceilf(rect.height / tile);

                for (int r = 0; r < totalRows; r++) {
                    bool isTop = (r == 0);
                    const Texture2D& tex = isTop ? theme.platformTop : theme.platformBody;
                    float texW = (float)tex.width;
                    float texH = (float)tex.height;

                    for (int c = 0; c < midCols; c++) {
                        float drawX = rect.x + edgeW + c * tile;
                        float drawY = rect.y + r * tile;
                        float drawW = tile;
                        float drawH = tile;
                        float srcW  = texW;
                        float srcH  = texH;

                        if (drawX + drawW > rect.x + edgeW + midW) {
                            float over = (drawX + drawW) - (rect.x + edgeW + midW);
                            srcW *= (drawW - over) / drawW;
                            drawW -= over;
                        }
                        if (drawY + drawH > rectBottom) {
                            float over = (drawY + drawH) - rectBottom;
                            srcH *= (drawH - over) / drawH;
                            drawH -= over;
                        }

                        Rectangle src = { 0.0f, 0.0f, srcW, srcH };
                        Rectangle dst = { drawX, drawY, drawW, drawH };
                        DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
                    }
                }
            }
            else {
                DrawRectangle(
                    (int)(rect.x + edgeW), (int)rect.y,
                    (int)midW, (int)rect.height,
                    DARKGRAY);
            }
        }

        DrawTextureTiledColumn(theme.platformLeft,
            rect.x, rect.y, rectBottom, tile, edgeW);

        DrawTextureTiledColumn(theme.platformRight,
            rect.x + rect.width - edgeW, rect.y, rectBottom, tile, edgeW);
    }
    else {
        int cols      = (int)ceilf(rect.width / tile);
        int totalRows = (int)ceilf(rect.height / tile);

        for (int r = 0; r < totalRows; r++) {
            bool isTop = (r == 0);
            const Texture2D& tex = isTop ? theme.platformTop : theme.platformBody;
            float texW = (float)tex.width;
            float texH = (float)tex.height;

            for (int c = 0; c < cols; c++) {
                float drawX = rect.x + c * tile;
                float drawY = rect.y + r * tile;
                float drawW = tile;
                float drawH = tile;
                float srcW  = texW;
                float srcH  = texH;

                if (drawX + drawW > rect.x + rect.width) {
                    float over = (drawX + drawW) - (rect.x + rect.width);
                    srcW *= (drawW - over) / drawW;
                    drawW -= over;
                }
                if (drawY + drawH > rect.y + rect.height) {
                    float over = (drawY + drawH) - (rect.y + rect.height);
                    srcH *= (drawH - over) / drawH;
                    drawH -= over;
                }

                Rectangle src = { 0.0f, 0.0f, srcW, srcH };
                Rectangle dst = { drawX, drawY, drawW, drawH };
                DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
            }
        }
    }
}