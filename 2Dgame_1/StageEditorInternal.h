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
struct ParamDef { 
	const char* name;//パラメータ名
	float defaultValue;//デフォルト値
	bool isBool;//bool型かどうか
};//パラメータ定義

struct TypeParamInfo { 
	int count;//このオブジェクトタイプのパラメータ数 
	ParamDef defs[MAX_OBJ_PARAMS];//パラメータ定義配列
};//オブジェクトタイプのパラメータ情報

//敵タイプ別パラメータ情報
struct EnemyTypeParamInfo {
	int count;
	ParamDef defs[MAX_OBJ_PARAMS];
};

// プロパティパネル定数
constexpr float PROP_W = 280.0f;
constexpr float PROP_LINE_H = 28.0f;
constexpr float PROP_HEADER_H = 58.0f;

// ================================================================
// 共有ヘルパー関数 (StageEditor.cpp で定義)
// ================================================================
const TypeParamInfo& EdGetTypeInfo(EditorObjectType t);//オブジェクトタイプに応じたパラメータ情報を取得する
void        InitDefaultParams(PlacedObject& obj);//	オブジェクトタイプに応じたデフォルトパラメータを初期化する
const char* GetNameEN(int i);//オブジェクトタイプに応じた英語名を取得する
const char* GetNameJP(int i);//オブジェクトタイプに応じた名前を取得する
Color       GetColor(int i);//オブジェクトタイプに応じた色を取得する
Rectangle   GetPropPanelRect(const StageEditor& ed);//プロパティパネルの矩形を取得する
Rectangle   GetBtnRect(const StageEditor& ed, int visibleIdx);//ツールバーのボタン矩形を取得する
void        AdjustToolbarOffset(StageEditor& ed);//ツールバーのオフセットを調整する（ボタン数に応じて中央寄せ）
void        DrawObjectIcon(int typeIdx, Rectangle r);//オブジェクトアイコンを描画する
void        DrawPropertyPanel(const StageEditor& ed);//プロパティパネルを描画する
Vector2     SnapToGrid(Vector2 world, float grid);//ワールド座標をグリッドにスナップする
std::string FitLabelToWidth(const char* text, int fontSize, float maxWidth);//テキストを指定幅に収まるように切り詰める（末尾に "..." を付加）

const EnemyTypeParamInfo& EdGetEnemyTypeInfo(EnemyType t);
void                      InitDefaultEnemyParams(PlacedEnemy& enemy);//敵のタイプに応じたデフォルトパラメータを初期化する
int                       GetEnemyAtWorldPos(const StageEditor& ed, Vector2 worldPos);//敵の配置位置を取得する
Rectangle                 GetEnemyRect(const PlacedEnemy& enemy);//敵の矩形を取得する
void                      EditorAddEnemy(StageEditor& ed, EnemyType type,Vector2 worldPos);//敵を追加する
void                      EditorRemoveEnemy(StageEditor& ed, int enemyIdx);//敵を削除する
void                      EditorSetEnemyParam(StageEditor& ed, int enemyIdx, int paramIdx, float value);//敵のパラメータを設定する

void DrawEnemyPropertyPanel(StageEditor& ed);//敵のプロパティパネルを描画する)