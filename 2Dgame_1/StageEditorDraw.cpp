#include "StageEditorInternal.h"
#include "DialogManager.h"  
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
        DrawRectangleRec(obj.rect, ColorAlpha(GetColor(t), 0.7f));
        DrawRectangleLinesEx(obj.rect, 2, BLACK);
        DrawText(GetNameEN(t), (int)obj.rect.x + 2, (int)obj.rect.y + 2, 10, BLACK);
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
void EditorDrawUI(const StageEditor& ed) {
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

    DrawPropertyPanel(ed);
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
        } else if (editing) {
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
}