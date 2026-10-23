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
// 指定したオブジェクトタイプが持つ「編集可能パラメータ定義」を返す
const TypeParamInfo& EdGetTypeInfo(EditorObjectType t);
// オブジェクト生成時に、タイプごとのデフォルト値を params に設定する
void        InitDefaultParams(PlacedObject& obj);
// ツールバー表示用の英語ラベルを取得する
const char* GetNameEN(int i);
// UI表示用の日本語ラベルを取得する
const char* GetNameJP(int i);
// タイプごとの識別色（エディタ描画用）を取得する
Color       GetColor(int i);
// 右側プロパティパネルの表示範囲（位置とサイズ）を計算する
Rectangle   GetPropPanelRect(const StageEditor& ed);
// 表示中ツールバーの n 番目ボタン矩形を取得する
Rectangle   GetBtnRect(const StageEditor& ed, int visibleIdx);
// 現在選択中タイプが見切れないようにツールバーの表示オフセットを補正する
void        AdjustToolbarOffset(StageEditor& ed);
// ツールバー内で使う簡易アイコンを描画する
void        DrawObjectIcon(int typeIdx, Rectangle r);
// 選択中オブジェクトのパラメータ編集パネルを描画する
void        DrawPropertyPanel(const StageEditor& ed);
// ワールド座標を現在のグリッド単位へ丸める
Vector2     SnapToGrid(Vector2 world, float grid);
// 長いラベル文字列を指定幅に収まるように省略表示する
std::string FitLabelToWidth(const char* text, int fontSize, float maxWidth);

// 敵タイプごとの編集パラメータ定義を返す
const EnemyTypeParamInfo& EdGetEnemyTypeInfo(EnemyType t);
// 敵配置データをタイプごとのデフォルト値で初期化する
void                      InitDefaultEnemyParams(PlacedEnemy& enemy);
// 指定座標に重なっている敵のインデックスを取得する（未ヒット時は-1）
int                       GetEnemyAtWorldPos(const StageEditor& ed, Vector2 worldPos);
// 敵の当たり判定矩形（エディタ選択用）を返す
Rectangle                 GetEnemyRect(const PlacedEnemy& enemy);
// 新しい敵を配置リストへ追加する
void                      EditorAddEnemy(StageEditor& ed, EnemyType type,Vector2 worldPos);
// 指定インデックスの敵を削除し、選択状態も整合させる
void                      EditorRemoveEnemy(StageEditor& ed, int enemyIdx);
// 敵パラメータ1項目を更新する
void                      EditorSetEnemyParam(StageEditor& ed, int enemyIdx, int paramIdx, float value);

// 敵専用のプロパティ編集パネルを描画する
void DrawEnemyPropertyPanel(StageEditor& ed);
