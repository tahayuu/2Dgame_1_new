#pragma once
#include <string>
#include "GameEvents.h"

// ================================================================
// OjisanDialog.h の役割
// ---------------------------------------------------------------
// ・死亡文脈(OjisanContext)から、おじさん台詞を引くAPIを定義する。
// ・DialogManager のキー設計と連携して文言を切り替える。
// ================================================================

struct OjisanContext {
	DeathCause cause = DeathCause::UNKNOWN;
	std::string areaName = "";
	std::string stageName = "";
	std::string enemyName = "";
	int totalDeaths = 0;
	bool hasItem = false;
	float playTime = 0.0f;
};
// 死因に対する Ojisan のセリフを返す。
// 呼び出し側は DialogManager を事前にロードしていることを想定。

std::string GetOjiSanLine(const OjisanContext& ctx);