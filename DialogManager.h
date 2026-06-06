// 2Dgame_1\DialogManager.h
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class DialogManager {
public:
    // シングルトン取得
    static DialogManager& Instance();

    // デフォルト/明示読み込み
    void LoadFromFile(const std::string& path);
    void Reload(); // 最後に使ったパスでリロード

    // 取得 API（自動ロードあり）
    std::string Get(const std::string& id, const std::string& fallback = "") const;
    std::string GetRandom() const;
    std::string GetRandomByPrefix(const std::string& prefix) const;

    // ユーティリティ
    std::vector<std::string> Keys() const;
    bool Has(const std::string& id) const;

    // 文字列フォーマット（簡易）: "{0}" "{1}" ... を args で置換
    static std::string Format(const std::string& text, const std::vector<std::string>& args);

private:
    DialogManager(); // private ctor
    std::unordered_map<std::string, std::string> lines;
    std::string lastPath;
};