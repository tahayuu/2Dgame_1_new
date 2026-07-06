#include "StageEditorInternal.h"
#include "DialogManager.h"  

// ================================================================
static Texture2D gEditorArrowTex{};
static bool gEditorArrowLoaded = false;

static void EnsureEditorArrowLoaded() {
    if (!gEditorArrowLoaded) {
        gEditorArrowTex = LoadTexture("assets/images/arrow.png");
        gEditorArrowLoaded = true;
    }
}

static void DrawEditorArrowTexture(Rectangle rect, float angleDeg) {
    EnsureEditorArrowLoaded();

    if (gEditorArrowTex.id == 0) {
        DrawRectangleRec(rect, GREEN);
        DrawRectangleLinesEx(rect, 2, DARKGREEN);
        return;
    }

    Rectangle src = {
        0.0f, 0.0f,
        (float)gEditorArrowTex.width,
        (float)gEditorArrowTex.height
    };

    Rectangle dst = {
        rect.x + rect.width * 0.5f,
        rect.y + rect.height * 0.5f,
        rect.width,
        rect.height
    };

    Vector2 origin = {
        rect.width * 0.5f,
        rect.height * 0.5f
    };

    DrawTexturePro(gEditorArrowTex, src, dst, origin, angleDeg, WHITE);
}
// ================================================================
void EditorDraw(const StageEditor& ed) {
    if (!ed.active) return;
    float grid = ed.gridSize;
    float left = ed.camera.target.x - ed.camera.offset.x / ed.camera.zoom - grid;
    float right = ed.camera.target.x + ed.camera.offset.x / ed.camera.zoom + grid;
    float top = ed.camera.target.y - ed.camera.offset.y / ed.camera.zoom - grid;
    float bottom = ed.camera.target.y + ed.camera.offset.y / ed.camera.zoom + grid;

    for (float x = floorf(left / grid) * grid; x <= right; x += grid)
        DrawLineV({ x, top }, { x, bottom }, ColorAlpha(DARKGRAY, 0.15f));
    for (float y = floorf(top / grid) * grid; y <= bottom; y += grid)
        DrawLineV({ left, y }, { right, y }, ColorAlpha(DARKGRAY, 0.15f));
    DrawLineV({ 0, top }, { 0, bottom }, ColorAlpha(GREEN, 0.4f));
    DrawLineV({ left, 0 }, { right, 0 }, ColorAlpha(RED, 0.4f));


    for (const auto& obj : ed.objects) {
        int t = (int)obj.type;

        // ================================================================
        // 見た目(SpriteId)が設定されている場合はスプライト描画を優先する。
        // ・当たり判定(obj.rect)やギミック種別(obj.type)には一切影響しない。
        // ・spriteId が None のときは、今まで通りの各ギミックの仮描画を使う
        //   （既存ギミックの見た目を壊さないため）。
        // ================================================================
        if (obj.spriteId != SpriteId::None) {
            SpriteDatabase::DrawSprite(obj.spriteId, obj.rect, obj.rotation,
                obj.flipX, obj.flipY, WHITE);
            // 選択中かどうか分かりやすいように、枠線だけは重ねて描く
            DrawRectangleLinesEx(obj.rect, 1, ColorAlpha(WHITE, 0.5f));
        }
        else if (obj.type == EditorObjectType::DECOR_ARROW) {
            DrawRectangleRec(obj.rect, ColorAlpha(GetColor(t), 0.15f));
            DrawRectangleLinesEx(obj.rect, 2, BLACK);
            DrawEditorArrowTexture(obj.rect, obj.params[0]);
            DrawText(GetNameEN(t), (int)obj.rect.x + 2, (int)obj.rect.y + 2, 10, BLACK);
        }
        else {
            DrawRectangleRec(obj.rect, ColorAlpha(GetColor(t), 0.7f));
            DrawRectangleLinesEx(obj.rect, 2, BLACK);
            DrawText(GetNameEN(t), (int)obj.rect.x + 2, (int)obj.rect.y + 2, 10, BLACK);
        }
    }

    for (int i = 0; i < (int)ed.placedEnemies.size(); i++) {
        const auto& enemy = ed.placedEnemies[i];
        const float R = 20.0f;

        Color typeColor = WHITE;
        switch (enemy.type) {
        case EnemyType::WALKER:   typeColor = { 255, 150,  50, 255 }; break; // オレンジ
        case EnemyType::FLYER:    typeColor = { 100, 150, 255, 255 }; break; // 青
        case EnemyType::SHOOTER:  typeColor = { 255,  80,  80, 255 }; break; // 赤
        case EnemyType::JUMPCOPY: typeColor = { 100, 220, 100, 255 }; break; // 緑
        }

        DrawCircleV(enemy.pos, R, typeColor);
        DrawCircleLinesV(enemy.pos, R, (i == ed.selectedEnemyIdx) ? YELLOW : ColorAlpha(BLACK, 0.6f));

        const char* label = "W";
        switch (enemy.type) {
        case EnemyType::WALKER:   label = "W"; break;
        case EnemyType::FLYER:    label = "F"; break;
        case EnemyType::SHOOTER:  label = "S"; break;
        case EnemyType::JUMPCOPY: label = "J"; break;
        }
        DrawText(label, (int)(enemy.pos.x - 5), (int)(enemy.pos.y - 7), 14, BLACK);

        // 選択中は太い外枠を追加
        if (i == ed.selectedEnemyIdx) {
            DrawCircleLinesV(enemy.pos, R + 4.0f, YELLOW);
        }
    }

    Vector2 mp = GetMousePosition();
    if (mp.y >= ed.TOOLBAR_H && mp.y <= ed.screenH - ed.BOTTOM_H) {
        Vector2 mw = GetScreenToWorld2D(mp, ed.camera);
        Vector2 sn = SnapToGrid(mw, grid);
        Rectangle pv = { sn.x, sn.y, grid * ed.gridW, grid * ed.gridH };
        Color pc = GetColor((int)ed.currentType);
        DrawRectangleRec(pv, ColorAlpha(pc, 0.35f));
        DrawRectangleLinesEx(pv, 2, ColorAlpha(pc, 0.8f));
    }

    if (ed.propSelectedIdx >= 0 && ed.propSelectedIdx < (int)ed.objects.size())
        DrawRectangleLinesEx(ed.objects[ed.propSelectedIdx].rect, 3, YELLOW);
}

// ================================================================
void EditorDrawUI(StageEditor& ed) {
    if (!ed.active) return;
    bool hasFont = (ed.uiFont.texture.id != 0);
    const int TC = (int)EditorObjectType::COUNT;

    DrawRectangle(0, 0, ed.screenW, (int)ed.TOOLBAR_H, { 30, 30, 42, 245 });
    DrawLineEx({ 0, ed.TOOLBAR_H }, { (float)ed.screenW, ed.TOOLBAR_H }, 2, ColorAlpha(WHITE, 0.25f));

    const int panelX = 10, panelY = 6, panelW = 420, panelH = 105;
    DrawRectangle(panelX, panelY, panelW, panelH, ColorAlpha(BLACK, 0.6f));
    DrawRectangleLinesEx({ (float)panelX, (float)panelY, (float)panelW, (float)panelH },
        1, ColorAlpha(WHITE, 0.3f));

    int lineY = panelY + 5;
    const int lineSpacing = 14;

    if (hasFont) {
        DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(u8"\u25A0 \u30A8\u30C7\u30A3\u30BF\u64CD\u4F5C"),
            { (float)panelX + 5, (float)lineY }, 13, 1, YELLOW);
        lineY += lineSpacing + 2;
        // 配置: 左クリック  削除: 右クリック
        DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(
            u8"\u914D\u7F6E: \u5DE6\u30AF\u30EA\u30C3\u30AF  \u524A\u9664: \u53F3\u30AF\u30EA\u30C3\u30AF"),
            { (float)panelX + 8, (float)lineY }, 11, 1, LIGHTGRAY);
        lineY += lineSpacing;
        // パラメータ編集: T  選択解除: Esc/Delete
        DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(
            u8"\u30D1\u30E9\u30E1\u30FC\u30BF\u7DE8\u96C6: T  \u9078\u629E\u89E3\u9664: Esc/Delete"),
            { (float)panelX + 8, (float)lineY }, 11, 1, LIGHTGRAY);
        lineY += lineSpacing;
        DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(
            u8"\u30BA\u30FC\u30E0: \u30DE\u30A6\u30B9\u30DB\u30A4\u30FC\u30EB(\u753B\u9762\u4E0A)"),
            { (float)panelX + 8, (float)lineY }, 11, 1, LIGHTGRAY);
        lineY += lineSpacing;
        DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(
            u8"\u30B5\u30A4\u30BA: Q/W(\u5E45) A/S(\u9AD8\u3055)  \u30B0\u30EA\u30C3\u30C9: R"),
            { (float)panelX + 8, (float)lineY }, 11, 1, LIGHTGRAY);
        lineY += lineSpacing;
        DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(
            u8"\u79FB\u52D5: \u2190\u2192\u2191\u2193  \u4FDD\u5B58: Ctrl+S  \u53D6\u6D88: Ctrl+Z"),
            { (float)panelX + 8, (float)lineY }, 11, 1, LIGHTGRAY);
    }
    else {
        DrawText("EDITOR CONTROLS", panelX + 5, lineY, 11, YELLOW);
        lineY += lineSpacing + 2;
        DrawText("Place: LClick  Delete: RClick", panelX + 8, lineY, 10, LIGHTGRAY);
        lineY += lineSpacing;
        DrawText("Params: T  Deselect: Esc/Del  Zoom: Wheel", panelX + 8, lineY, 10, LIGHTGRAY);
        lineY += lineSpacing;
        DrawText("Size: Q/W  Grid: R  Save: Ctrl+S  Undo: Ctrl+Z", panelX + 8, lineY, 10, LIGHTGRAY);
    }

    float totalW = ed.VISIBLE_BTNS * ed.BTN_W + (ed.VISIBLE_BTNS - 1) * ed.BTN_GAP;
    float barSX = (ed.screenW - totalW) / 2.0f;
    if (ed.toolbarOffset > 0)
        DrawText("<", (int)barSX - 20, (int)(ed.TOOLBAR_H / 2 - 8), 20, WHITE);
    if (ed.toolbarOffset + ed.VISIBLE_BTNS < TC)
        DrawText(">", (int)(barSX + totalW + 6), (int)(ed.TOOLBAR_H / 2 - 8), 20, WHITE);
    DrawText(TextFormat("%d/%d", (int)ed.currentType + 1, TC),
        (int)(barSX + totalW + 28), (int)(ed.TOOLBAR_H / 2 - 6), 12, GRAY);

    for (int v = 0; v < ed.VISIBLE_BTNS; v++) {
        int typeIdx = ed.toolbarOffset + v;
        if (typeIdx >= TC) break;
        Rectangle btn = GetBtnRect(ed, v);
        bool selected = (typeIdx == (int)ed.currentType);
        bool hovered = CheckCollisionPointRec(GetMousePosition(), btn);

        Color bg = selected ? Color{ 75,75,115,255 }
            : hovered ? Color{ 55,55,75,255 }
        : Color{ 42,42,52,255 };
        DrawRectangleRounded(btn, 0.15f, 4, bg);

        if (selected) {
            Color tc = GetColor(typeIdx);
            DrawRectangle((int)btn.x + 2, (int)btn.y - 5, (int)btn.width - 4, 5, tc);
        }

        Rectangle iconR = { btn.x + (btn.width - 44) / 2, btn.y + 5, 44, 20 };
        DrawObjectIcon(typeIdx, iconR);

        {
            std::string label = FitLabelToWidth(GetNameEN(typeIdx), 9, btn.width - 8.0f);
            int tw = MeasureText(label.c_str(), 9);
            DrawText(label.c_str(),
                (int)(btn.x + (btn.width - tw) / 2),
                (int)(btn.y + btn.height - 14), 9, WHITE);
        }

        if (selected)
            DrawRectangleRoundedLinesEx(btn, 0.15f, 4, 2, YELLOW);
        else if (hovered)
            DrawRectangleRoundedLinesEx(btn, 0.15f, 4, 1, ColorAlpha(WHITE, 0.4f));
    }

    float by = (float)ed.screenH - ed.BOTTOM_H;
    DrawRectangle(0, (int)by, ed.screenW, (int)ed.BOTTOM_H, { 30,30,42,230 });
    DrawLineEx({ 0, by }, { (float)ed.screenW, by }, 1, ColorAlpha(WHITE, 0.2f));

    float ty = by + 9;
    if (hasFont) {
        float x = 10;
        const char* sel = GetNameJP((int)ed.currentType);
        DrawTextEx(ed.uiFont, sel, { x, ty }, 17, 1, GetColor((int)ed.currentType));
        x += MeasureTextEx(ed.uiFont, sel, 17, 1).x + 15;
        DrawText(TextFormat("Grid:%.0f  Size:%dx%d  Zoom:%.1fx  Obj:%d",
            ed.gridSize, ed.gridW, ed.gridH, ed.camera.zoom, (int)ed.objects.size()),
            (int)x, (int)ty + 2, 13, LIGHTGRAY);
    }
    else {
        DrawText(TextFormat("%s | Grid:%.0f Size:%dx%d | Zoom:%.1fx | Obj:%d",
            GetNameEN((int)ed.currentType), ed.gridSize, ed.gridW, ed.gridH,
            ed.camera.zoom, (int)ed.objects.size()),
            10, (int)ty, 13, LIGHTGRAY);
    }

    if (ed.saveNotifyTimer > 0.0f) {
        float a = (ed.saveNotifyTimer > 1.0f) ? 1.0f : ed.saveNotifyTimer;
        DrawText("Saved!", ed.screenW / 2 - 30, (int)ed.TOOLBAR_H + 12, 24, ColorAlpha(GREEN, a));
    }

    // ================================================================
    // === 左下に操作ガイドを表示 ===
    // ================================================================
    {
        const float guideX = 10.0f;
        const float guideY = ed.screenH - 220.0f; // 下部から十分な距離
        const float guideW = 280.0f;
        const float guideH = 210.0f;

        DrawRectangleRounded({guideX, guideY, guideW, guideH}, 0.08f, 4, ColorAlpha(BLACK, 0.7f));
        DrawRectangleRoundedLinesEx({guideX, guideY, guideW, guideH}, 0.08f, 4, 1, ColorAlpha(WHITE, 0.3f));

        float gy = guideY + 6.0f;
        const float lineH = 13.0f;

        if (hasFont) {
            DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(u8"■ 操作ガイド"),
                {guideX + 8, gy}, 11, 1, YELLOW);
        } else {
            DrawText("QUICK GUIDE", (int)guideX + 8, (int)gy, 10, YELLOW);
        }
        gy += lineH + 2;

        // 基本操作
        if (hasFont) {
            DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(u8"■配置: 左クリック"),
                {guideX + 8, gy}, 10, 1, LIGHTGRAY); gy += lineH;
            DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(u8"■選択: T キー"),
                {guideX + 8, gy}, 10, 1, LIGHTGRAY); gy += lineH;
            DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(u8"■削除: 右クリック / Del"),
                {guideX + 8, gy}, 10, 1, LIGHTGRAY); gy += lineH + 2;

            // コピー＆ペースト
            DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(u8"■コピー: C キー (選択時)"),
                {guideX + 8, gy}, 10, 1, LIGHTGRAY); gy += lineH;
            DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(u8"■貼付: P キー"),
                {guideX + 8, gy}, 10, 1, LIGHTGRAY); gy += lineH + 2;

            // サイズ変更
            DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(u8"■大きくする: + キー"),
                {guideX + 8, gy}, 10, 1, LIGHTGRAY); gy += lineH;
            DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(u8"■小さくする: - キー"),
                {guideX + 8, gy}, 10, 1, LIGHTGRAY); gy += lineH + 2;

            // スプライト操作
            DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(u8"■見た目切替: [ / ] キー"),
                {guideX + 8, gy}, 10, 1, LIGHTGRAY); gy += lineH;
            DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(u8"■反転: H(左右) J(上下)"),
                {guideX + 8, gy}, 10, 1, LIGHTGRAY); gy += lineH;
            DrawTextEx(ed.uiFont, reinterpret_cast<const char*>(u8"■回転: K(反時計) L(時計)"),
                {guideX + 8, gy}, 10, 1, LIGHTGRAY);
        } else {
            DrawText("Place: LClick", (int)guideX + 8, (int)gy, 9, LIGHTGRAY); gy += lineH;
            DrawText("Select: T | Delete: RClick/Del", (int)guideX + 8, (int)gy, 9, LIGHTGRAY); gy += lineH + 2;
            DrawText("Copy: C | Paste: P", (int)guideX + 8, (int)gy, 9, LIGHTGRAY); gy += lineH;
            DrawText("Size: + / -", (int)guideX + 8, (int)gy, 9, LIGHTGRAY); gy += lineH + 2;
            DrawText("Sprite: [ ] | Flip: H/J", (int)guideX + 8, (int)gy, 9, LIGHTGRAY); gy += lineH;
            DrawText("Rotate: K/L", (int)guideX + 8, (int)gy, 9, LIGHTGRAY);
        }
    }

    DrawPropertyPanel(ed);        // 通常オブジェクトのパネル（既存）
    DrawEnemyPropertyPanel(ed);   // ← 敵用パネル（新規追加）


    // ===== ENEMYタイプ選択中のときのサイドパネル =====
    if (ed.currentType == EditorObjectType::ENEMY) {
        const float PX = 10.0f;
        const float PY = ed.TOOLBAR_H + 10.0f;
        const float PW = 130.0f;
        const float PH = 175.0f;

        DrawRectangleRounded({PX, PY, PW, PH}, 0.1f, 4, {30, 30, 50, 230});
        DrawRectangleRoundedLinesEx({PX, PY, PW, PH}, 0.1f, 4, 2, ColorAlpha(ORANGE, 0.7f));
        DrawText("ENEMY TYPE", (int)PX + 6, (int)PY + 6, 11, ORANGE);

        // 敵タイプボタン
        const char* typeNames[] = {"WALKER","FLYER","SHOOTER","JUMPCOPY"};
        Color typeColors[] = {
            {255, 150, 50, 255},
            {100, 150, 255, 255},
            {255, 80, 80, 255},
            {100, 220, 100, 255},
        };

        for (int i = 0; i < 4; i++) {
            Rectangle btn = {PX + 5, PY + 28 + i * 34, PW - 10, 28};
            bool isSelected = (ed.currentEnemyType == (EnemyType)i);
            bool isHover    = CheckCollisionPointRec(GetMousePosition(), btn);

            Color bg = isSelected ? ColorAlpha(typeColors[i], 0.85f)
                     : isHover    ? ColorAlpha(typeColors[i], 0.5f)
                                  : ColorAlpha(typeColors[i], 0.3f);
            DrawRectangleRounded(btn, 0.2f, 4, bg);
            DrawRectangleRoundedLinesEx(btn, 0.2f, 4, isSelected ? 2 : 1,
                isSelected ? YELLOW : ColorAlpha(WHITE, 0.4f));
            DrawText(typeNames[i], (int)(btn.x + 5), (int)(btn.y + 7), 11, WHITE);

            // クリックで敵タイプを選択
            if (isHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                ed.currentEnemyType = (EnemyType)i;
            }
        }

        // 操作説明
        const float GY = PY + PH + 8.0f;
        DrawText("LClick: Place", (int)PX, (int)GY,      10, LIGHTGRAY);
        DrawText("RClick: Delete",(int)PX, (int)GY + 14, 10, LIGHTGRAY);
        DrawText("T: Select",     (int)PX, (int)GY + 28, 10, LIGHTGRAY);
        DrawText("Del: Remove",   (int)PX, (int)GY + 42, 10, LIGHTGRAY);
    }
}

// ================================================================
void DrawPropertyPanel(const StageEditor& ed) {
    if (ed.propSelectedIdx < 0 || ed.propSelectedIdx >= (int)ed.objects.size()) return;

    const auto& sel = ed.objects[ed.propSelectedIdx];
    const auto& info = EdGetTypeInfo(sel.type);
    bool hasFont = (ed.uiFont.texture.id != 0);

    // コメントブロックはテキスト行が1行追加されるのでパネルを拡張
    bool isComment = (sel.type == EditorObjectType::COMMENT_BLOCK);
    Rectangle panel = GetPropPanelRect(ed);

    DrawRectangleRounded(panel, 0.08f, 4, { 20, 20, 40, 235 });
    DrawRectangleRoundedLinesEx(panel, 0.08f, 4, 2, ColorAlpha(WHITE, 0.4f));

    float y = panel.y + 8;
    Color tc = GetColor((int)sel.type);
    if (hasFont)
        DrawTextEx(ed.uiFont, GetNameJP((int)sel.type), { panel.x + 10, y }, 18, 1, tc);
    else
        DrawText(GetNameEN((int)sel.type), (int)panel.x + 10, (int)y, 16, tc);
    y += 24;

    DrawText(TextFormat("x:%.0f y:%.0f w:%.0f h:%.0f",
        sel.rect.x, sel.rect.y, sel.rect.width, sel.rect.height),
        (int)panel.x + 10, (int)y, 11, LIGHTGRAY);
    y += 18;

    DrawLineEx({ panel.x + 8, y }, { panel.x + panel.width - 8, y }, 1, ColorAlpha(WHITE, 0.2f));
    y += 8;

    Vector2 mousePos = GetMousePosition();
    for (int i = 0; i < info.count; i++) {
        Rectangle row = { panel.x + 5, y, panel.width - 10, PROP_LINE_H };
        bool hov = CheckCollisionPointRec(mousePos, row);
        bool editing = (ed.propEditingParam == i);

        if (hov && !editing) DrawRectangleRec(row, ColorAlpha(WHITE, 0.06f));
        DrawText(info.defs[i].name, (int)panel.x + 12, (int)y + 7, 12, WHITE);

        float valX = panel.x + 160;

        if (info.defs[i].isBool) {
            bool bval = (sel.params[i] != 0.0f);
            Color btnCol = bval ? Color{50,140,50,255} : Color{140,50,50,255};
            if (hov) btnCol = bval ? Color{70,180,70,255} : Color{180,70,70,255};
            Rectangle toggleR = { valX - 4, y + 3, 60, PROP_LINE_H - 6 };
            DrawRectangleRounded(toggleR, 0.3f, 4, btnCol);
            const char* txt = bval ? "ON" : "OFF";
            int tw = MeasureText(txt, 11);
            DrawText(txt, (int)(toggleR.x + (toggleR.width - tw) / 2), (int)y + 8, 11, WHITE);
        }
        // EXIT_DOOR の targetStage パラメータの特別処理
        else if (sel.type == EditorObjectType::EXIT_DOOR && strcmp(info.defs[i].name, "targetStage") == 0) {
            int stageIdx = (int)sel.params[i];
            if (stageIdx < 0) stageIdx = 0;
            if (stageIdx > 5) stageIdx = 5;  // ステージ数は6（0～5）
            const char* stageNames[] = { "Stage 0", "Stage 1", "Stage 2", "Stage 3", "Stage 4", "Stage 5" };

            // ドロップダウン風表示
            Color ddBgCol = hov ? Color{60,100,140,255} : Color{40,80,120,255};
            Rectangle ddRect = { valX - 4, y + 3, panel.width - 168, PROP_LINE_H - 6 };
            DrawRectangleRounded(ddRect, 0.2f, 3, ddBgCol);
            DrawRectangleRoundedLinesEx(ddRect, 0.2f, 3, 1, SKYBLUE);
            DrawText(stageNames[stageIdx], (int)valX, (int)y + 7, 12, WHITE);
            DrawText("v", (int)(ddRect.x + ddRect.width - 12), (int)y + 7, 12, LIGHTGRAY);
        }
        else if (editing) {
            Rectangle editR = { valX - 4, y + 2, panel.width - 168, PROP_LINE_H - 4 };
            DrawRectangleRec(editR, { 40, 40, 60, 255 });
            DrawRectangleLinesEx(editR, 1, YELLOW);
            DrawText(ed.propEditBuf, (int)valX, (int)y + 7, 12, YELLOW);
            if ((int)(GetTime() * 2) % 2 == 0) {
                int tw = MeasureText(ed.propEditBuf, 12);
                DrawText("|", (int)valX + tw, (int)y + 6, 12, YELLOW);
            }
        } else {
            DrawText(TextFormat("%.2f", sel.params[i]), (int)valX, (int)y + 7, 12, SKYBLUE);
        }
        y += PROP_LINE_H;
    }

    if (info.count == 0) { DrawText("(no params)", (int)panel.x + 12, (int)y + 7, 11, GRAY); y += PROP_LINE_H; }

    // === コメントブロック テキスト行 ===
    if (isComment) {
        y += 4;
        Rectangle textRow = { panel.x + 5, y, panel.width - 10, PROP_LINE_H };
        bool hov = CheckCollisionPointRec(mousePos, textRow);
        if (hov && !ed.propEditingText) DrawRectangleRec(textRow, ColorAlpha(WHITE, 0.06f));
        DrawText("text", (int)panel.x + 12, (int)y + 7, 12, WHITE);

        float valX = panel.x + 60;
        if (ed.propEditingText) {
            // --- 編集中 ---
            Rectangle editR = { valX - 4, y + 2, panel.width - 68, PROP_LINE_H - 4 };
            DrawRectangleRec(editR, { 40, 40, 60, 255 });
            DrawRectangleLinesEx(editR, 1, YELLOW);
            if (hasFont)
                DrawTextEx(ed.uiFont, ed.propTextBuf, { valX, y + 5 }, 13, 1, YELLOW);
            else
                DrawText(ed.propTextBuf, (int)valX, (int)y + 7, 12, YELLOW);
            if ((int)(GetTime() * 2) % 2 == 0) {
                float tw = hasFont
                    ? MeasureTextEx(ed.uiFont, ed.propTextBuf, 13, 1).x
                    : (float)MeasureText(ed.propTextBuf, 12);
                DrawText("|", (int)(valX + tw), (int)y + 6, 12, YELLOW);
            }
            // ヒント
            if (hasFont)
                DrawTextEx(ed.uiFont,
                    reinterpret_cast<const char*>(u8"自由テキスト または @キー名 で入力"),
                    { panel.x + 8, y + PROP_LINE_H + 2 }, 9, 1, ColorAlpha(GRAY, 0.8f));
        } else {
            // --- 非編集中: @キー判定 ---
            const std::string& txt = sel.text;
            if (!txt.empty() && txt[0] == '@') {
                const std::string key = txt.substr(1);
                bool found = DialogManager::Instance().Has(key);
                Color keyCol = found ? Color{100, 220, 100, 255} : Color{220, 100, 100, 255};
                if (hasFont)
                    DrawTextEx(ed.uiFont, txt.c_str(), { valX, y + 5 }, 13, 1, keyCol);
                else
                    DrawText(txt.c_str(), (int)valX, (int)y + 7, 12, keyCol);
                const char* badge = found ? "[OK]" : "[?]";
                DrawText(badge,
                    (int)(panel.x + panel.width - 38), (int)y + 7, 10,
                    found ? GREEN : RED);
            } else {
                const char* dispText = txt.empty()
                    ? reinterpret_cast<const char*>(u8"（クリックして編集）")
                    : txt.c_str();
                Color dispCol = txt.empty() ? GRAY : SKYBLUE;
                if (hasFont)
                    DrawTextEx(ed.uiFont, dispText, { valX, y + 5 }, 13, 1, dispCol);
                else
                    DrawText(dispText, (int)valX, (int)y + 7, 12, dispCol);
            }
        }
        y += PROP_LINE_H;

        // --- プレビュー行（@キーモードのみ表示） ---
        {
            const std::string& txt = sel.text;
            if (!txt.empty() && txt[0] == '@') {
                const std::string key = txt.substr(1);
                std::string resolved = DialogManager::Instance().Has(key)
                    ? DialogManager::Instance().Get(key, "")
                    : reinterpret_cast<const char*>(u8"（キーが見つかりません）");
                constexpr int kMaxPreview = 26;
                if ((int)resolved.size() > kMaxPreview)
                    resolved = resolved.substr(0, kMaxPreview) + "...";
                DrawText("->", (int)panel.x + 10, (int)y + 5, 10, GRAY);
                Color prevCol = { 200, 200, 100, 255 };
                if (hasFont)
                    DrawTextEx(ed.uiFont, resolved.c_str(),
                        { panel.x + 30, y + 3 }, 11, 1, prevCol);
                else
                    DrawText(resolved.c_str(), (int)panel.x + 30, (int)y + 5, 10, prevCol);
            }
            y += PROP_LINE_H;
        }
    }

    // ================================================================
    // === Sprite(見た目) 情報表示欄 ===
    // ----------------------------------------------------------------
    // EditorObjectType(ギミック)とは別に、見た目専用の SpriteId /
    // rotation / flipX / flipY を表示する。
    // 変更はキー操作（[ ] H J K L）で行う。GetPropPanelRect() 側で
    // この表示分の高さを確保しているので、両方セットで変更すること。
    // ================================================================
    y += 4;
    DrawLineEx({ panel.x + 8, y }, { panel.x + panel.width - 8, y }, 1, ColorAlpha(WHITE, 0.2f));
    y += 8;

    // 見出し
    Color spriteHeadCol = (sel.spriteId != SpriteId::None) ? SKYBLUE : GRAY;
    if (hasFont)
        DrawTextEx(ed.uiFont,
            reinterpret_cast<const char*>(u8"見た目(Sprite)"),
            { panel.x + 10, y }, 13, 1, spriteHeadCol);
    else
        DrawText("Sprite", (int)panel.x + 10, (int)y, 12, spriteHeadCol);
    y += PROP_LINE_H;

    // 現在の SpriteId 名
    const char* spriteName = SpriteDatabase::GetSpriteName(sel.spriteId);
    DrawText(TextFormat("[ ] %s", spriteName),
        (int)panel.x + 12, (int)y + 2, 12, WHITE);
    y += PROP_LINE_H;

    // rotation / flipX / flipY の状態
    DrawText(TextFormat("K/L rot:%.0f  H:flipX(%s)  J:flipY(%s)",
        sel.rotation,
        sel.flipX ? "ON" : "OFF",
        sel.flipY ? "ON" : "OFF"),
        (int)panel.x + 12, (int)y + 2, 11, LIGHTGRAY);
    y += PROP_LINE_H;
}

// ================================================================
// 敵プロパティパネル
// ================================================================
void DrawEnemyPropertyPanel(StageEditor& ed) {
    if (ed.currentType != EditorObjectType::ENEMY) return;
    if (ed.selectedEnemyIdx < 0 ||
        ed.selectedEnemyIdx >= (int)ed.placedEnemies.size()) return;

    auto& enemy = ed.placedEnemies[ed.selectedEnemyIdx];
    const auto& info = EdGetEnemyTypeInfo(enemy.type);
    bool hasFont = (ed.uiFont.texture.id != 0);

    // パネル矩形
    float panelW = PROP_W;
    float panelH = PROP_HEADER_H + (float)info.count * PROP_LINE_H + 40.0f;
    float panelX = ed.screenW - panelW - 10.0f;
    float panelY = ed.TOOLBAR_H + 10.0f;
    Rectangle panel = { panelX, panelY, panelW, panelH };

    DrawRectangleRounded(panel, 0.08f, 4, { 20, 20, 40, 235 });
    DrawRectangleRoundedLinesEx(panel, 0.08f, 4, 2, ColorAlpha(ORANGE, 0.6f));

    // ヘッダー
    const char* typeName = "WALKER";
    switch (enemy.type) {
    case EnemyType::WALKER:   typeName = "WALKER";   break;
    case EnemyType::FLYER:    typeName = "FLYER";    break;
    case EnemyType::SHOOTER:  typeName = "SHOOTER";  break;
    case EnemyType::JUMPCOPY: typeName = "JUMPCOPY"; break;
    }
    DrawText(TextFormat("Enemy: %s", typeName), (int)panelX + 10, (int)panelY + 8, 16, ORANGE);
    DrawText(TextFormat("x:%.0f y:%.0f", enemy.pos.x, enemy.pos.y),
        (int)panelX + 10, (int)panelY + 28, 11, LIGHTGRAY);
    DrawLineEx({ panelX + 8, panelY + 48 }, { panelX + panelW - 8, panelY + 48 },
        1, ColorAlpha(WHITE, 0.2f));

    // パラメータ行
    float y = panelY + PROP_HEADER_H;
    Vector2 mp = GetMousePosition();
    for (int i = 0; i < info.count; i++) {
        Rectangle row = { panelX + 5, y, panelW - 10, PROP_LINE_H };
        bool hov = CheckCollisionPointRec(mp, row);
        bool editing = (ed.propEditingParam == i);

        if (hov && !editing) DrawRectangleRec(row, ColorAlpha(WHITE, 0.06f));
        DrawText(info.defs[i].name, (int)panelX + 12, (int)y + 7, 12, WHITE);

        float valX = panelX + 160.0f;
        if (editing) {
            Rectangle editR = { valX - 4, y + 2, panelW - 168, PROP_LINE_H - 4 };
            DrawRectangleRec(editR, { 40, 40, 60, 255 });
            DrawRectangleLinesEx(editR, 1, YELLOW);
            DrawText(ed.propEditBuf, (int)valX, (int)y + 7, 12, YELLOW);
            if ((int)(GetTime() * 2) % 2 == 0) {
                int tw = MeasureText(ed.propEditBuf, 12);
                DrawText("|", (int)valX + tw, (int)y + 6, 12, YELLOW);
            }
        }
        else {
            DrawText(TextFormat("%.2f", enemy.params[i]), (int)valX, (int)y + 7, 12, SKYBLUE);
        }

        // クリックで編集開始
        if (hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            ed.propEditingParam = i;
            snprintf(ed.propEditBuf, sizeof(ed.propEditBuf), "%.2f", enemy.params[i]);
            ed.propEditCursor = (int)strlen(ed.propEditBuf);
        }
        y += PROP_LINE_H;
    }

    // 削除ボタン
    float delY = panelY + panelH - 34.0f;
    Rectangle delBtn = { panelX + panelW / 2 - 50, delY, 100, 24 };
    bool delHov = CheckCollisionPointRec(mp, delBtn);
    DrawRectangleRec(delBtn, delHov ? Color{ 180, 50, 50, 255 } : Color{ 120, 30, 30, 255 });
    int tw = MeasureText("Delete", 12);
    DrawText("Delete", (int)(delBtn.x + (delBtn.width - tw) / 2), (int)(delBtn.y + 6), 12, WHITE);

    if (delHov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        EditorRemoveEnemy(ed, ed.selectedEnemyIdx);
    }
}