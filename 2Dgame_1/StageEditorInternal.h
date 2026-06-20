#pragma once
#include "StageEditor.h"
#include "Stage.h"
#include "raymath.h"
#include <string>
#include <algorithm>

#ifdef _WIN32
#ifdef DrawText
#undef DrawText
#endif
#endif

// ================================================================
// 共有内部型
// ================================================================
struct ParamDef { const char* name; float defaultValue; bool isBool; };//パラメータ定義
struct TypeParamInfo { int count; ParamDef defs[MAX_OBJ_PARAMS]; };//オブジェクトタイプのパラメータ情報

// プロパティパネル定数
constexpr float PROP_W = 280.0f;
constexpr float PROP_LINE_H = 28.0f;
constexpr float PROP_HEADER_H = 58.0f;

// ================================================================
// 共有ヘルパー関数 (StageEditor.cpp で定義)
// ================================================================
const TypeParamInfo& EdGetTypeInfo(EditorObjectType t);
void        InitDefaultParams(PlacedObject& obj);
const char* GetNameEN(int i);
const char* GetNameJP(int i);
Color       GetColor(int i);
Rectangle   GetPropPanelRect(const StageEditor& ed);
Rectangle   GetBtnRect(const StageEditor& ed, int visibleIdx);
void        AdjustToolbarOffset(StageEditor& ed);
void        DrawObjectIcon(int typeIdx, Rectangle r);
void        DrawPropertyPanel(const StageEditor& ed);
Vector2     SnapToGrid(Vector2 world, float grid);
std::string FitLabelToWidth(const char* text, int fontSize, float maxWidth);