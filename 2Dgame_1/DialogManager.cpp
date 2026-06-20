// 2Dgame_1\DialogManager.cpp
#include "DialogManager.h"
#include <fstream>
#include <sstream>
#include <random>
#include <ctime>
#include <algorithm>
#include "raylib.h"

    // --- helpers ---
static inline std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// --- DialogManager implementation ---
DialogManager& DialogManager::Instance() {
    static DialogManager inst;
    return inst;
}

DialogManager::DialogManager() {
    // default path (実行ディレクトリ基準)
    lastPath = "ojisan_lines.text";
}

void DialogManager::LoadFromFile(const std::string& path) {
    lines.clear();
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open()) {
        lastPath.clear();
        return;
    }

    std::string line;
    bool firstLine = true;
    while (std::getline(ifs, line)) {
        // CRLF 対策
        if (!line.empty() && line.back() == '\r') line.pop_back();

        // BOM を先頭行で除去
        if (firstLine) {
            firstLine = false;
            if (line.size() >= 3 &&
                (unsigned char)line[0] == 0xEF &&
                (unsigned char)line[1] == 0xBB &&
                (unsigned char)line[2] == 0xBF) {
                line.erase(0, 3);
            }
        }

        line = trim(line);
        if (line.empty()) continue;
        if (line.front() == '#') continue; // コメント

        size_t sep = line.find('|');
        if (sep == std::string::npos) continue;

        std::string id = trim(line.substr(0, sep));
        std::string text = trim(line.substr(sep + 1)); // trim を追加
        lines[id] = text;
    }

    lastPath = path;
}

void DialogManager::Reload() {
    if (!lastPath.empty()) LoadFromFile(lastPath);
}

std::string DialogManager::Get(const std::string& id, const std::string& fallback) const {
    if (lines.empty()) {
        // 自動ロード（実行時に Instance().LoadFromFile(...) を呼び出していない場合）
        const_cast<DialogManager*>(this)->LoadFromFile(lastPath);
    }
    auto it = lines.find(id);
    return it != lines.end() ? it->second : fallback;
}

std::string DialogManager::GetRandom() const {
    if (lines.empty()) {
        const_cast<DialogManager*>(this)->LoadFromFile(lastPath);
    }
    if (lines.empty()) return {};
    static std::mt19937_64 rng((unsigned)std::time(nullptr));
    std::uniform_int_distribution<size_t> dist(0, lines.size() - 1);
    auto it = lines.begin();
    std::advance(it, dist(rng));
    return it->second;
}

std::string DialogManager::GetRandomByPrefix(const std::string& prefix) const {
    if (lines.empty()) {
        const_cast<DialogManager*>(this)->LoadFromFile(lastPath);
    }
    std::vector<std::string> matches;
    matches.reserve(8);
    for (auto& p : lines) {
        if (p.first.rfind(prefix, 0) == 0) matches.push_back(p.second); // prefix match
    }
    if (matches.empty()) return {};
    static std::mt19937_64 rng((unsigned)std::time(nullptr));
    std::uniform_int_distribution<size_t> dist(0, matches.size() - 1);
    return matches[dist(rng)];
}

std::vector<std::string> DialogManager::Keys() const {
    std::vector<std::string> ks;
    ks.reserve(lines.size());
    for (auto& p : lines) ks.push_back(p.first);
    return ks;
}

bool DialogManager::Has(const std::string& id) const {
    return lines.find(id) != lines.end();
}

std::string DialogManager::Format(const std::string& text, const std::vector<std::string>& args) {
    std::string out = text;
    for (size_t i = 0; i < args.size(); ++i) {
        std::string key = "{" + std::to_string(i) + "}";
        size_t pos = 0;
        while ((pos = out.find(key, pos)) != std::string::npos) {
            out.replace(pos, key.length(), args[i]);
            pos += args[i].length();
        }
    }
    return out;
}