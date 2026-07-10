#include "OjisanDialog.h"
#include "DialogManager.h"
#include <unordered_map>
#include <string>
#include <vector>

// OjisanDialog.cpp の役割: 死亡原因/回数/場所に応じた台詞キーを組み立てる。
static std::unordered_map<int, int> deathCounts;

// ===== 死因を文字列に変換するヘルパー =====
static std::string CauseToString(DeathCause cause) {
    switch (cause) {
    case DeathCause::FALL:           return "fall";
	case DeathCause::FALL_HIGH:      return "fall_high";
    case DeathCause::SPIKE:          return "spike";
	case DeathCause::SPIKE_RISING:   return "spike_rising";
    case DeathCause::TRAP:           return "trap";
    case DeathCause::ENEMY_WALKER:   return "enemy_walker";
    case DeathCause::ENEMY_FLYER:    return "enemy_flyer";
    case DeathCause::ENEMY_SHOOTER:  return "enemy_shooter";
    case DeathCause::OJISAN_PUNCH:   return "ojisan_punch";
    default:                         return "unknown";
    }
}


// =============================================================
//  拡張版：状況(コンテキスト)に応じたセリフ選び
//  
//  考え方：「一番具体的なキー」から順に探す。
//  見つかったらそれを返す。見つからなければ次に広いキーを試す。
//
//  テキストファイルの書き方例：
//    death_fall_cave_3|おじさん「この洞窟で3回目か…学習しろ！」
//    death_fall_cave|おじさん「洞窟は暗くて危険じゃ」
//    death_fall_5|おじさん「5回も落ちるとは…」
//    death_fall|おじさん「また落ちたのか」
//    death_unknown|おじさん「何があった？」
// =============================================================

// 目的: コンテキストに応じて優先順で台詞キーを探索し、最適な1文を返す。
std::string GetOjiSanLine(const OjisanContext& ctx) {  
    DialogManager& dm = DialogManager::Instance();
	std::string causeName = CauseToString(ctx.cause);// 死因を文字列に変換
    
    //死因ごとのカウントを増やす
	int key = static_cast<int>(ctx.cause);//cause: OjisanContext構造体のメンバー。DeathCause列挙型（例: FALL = 1, SPIKE = 2）
    int count = ++deathCounts[key];
    
    std::vector<std::string> keysToTry;//std::vector<std::string>: 文字列（std::string）を複数個格納できる可変長配列（動的配列）。
    //死因 エリア 回数
   //   例: "death_fall_cave_3"
    if (!ctx.areaName.empty()) 
    { keysToTry.push_back("death_" + causeName + "" + ctx.areaName + "_" + std::to_string(count)); }

    // 死因 + ステージ + 回数
    //   例: "death_fall_stage2_3"
    if (!ctx.stageName.empty()) {
        keysToTry.push_back("death_" + causeName + "_" + ctx.stageName + "_" + std::to_string(count));
    }

    if (!ctx.areaName.empty()) {
		keysToTry.push_back("death_" + causeName + "_" + ctx.areaName);
    }

    // 死因 + ステージ（回数なし）
    //   例: "death_fall_stage2"
    if (!ctx.stageName.empty()) {
        keysToTry.push_back("death_" + causeName + "_" + ctx.stageName);
    }

    // 死因 + 回数
    //   例: "death_fall_3"
    keysToTry.push_back("death_" + causeName + "_" + std::to_string(count));

    // 死因のみ
    //   例: "death_fall"
    keysToTry.push_back("death_" + causeName);

    // 汎用フォールバック
    keysToTry.push_back("death_unknown");

    for(const std::string& tryKey :keysToTry){
        if (dm.Has(tryKey)) {
            std::string line = dm.Get(tryKey, "");
            if (!line.empty())return line;
    }
    }
    return std::string(reinterpret_cast<const char*>(u8"おじさん「うーん…大丈夫か？」"));

}