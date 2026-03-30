#include "difficulty_config.h"
#include <fstream>
#include <sstream>

namespace game_core {

DifficultyConfig::DifficultyConfig()
    : profile_count_(0)
    , default_id_(2) // 默认普通难度
{
    initDefaults();
}

void DifficultyConfig::initDefaults() {
    // 内置四个难度，无需 JSON 文件也能运行
    profiles_[0] = {1, "简单",  2,  500, 0.30, 0.5, 0.5, "30%概率走随机步"};
    profiles_[1] = {2, "普通",  4, 1000, 0.10, 0.6, 0.6, "10%概率走随机步"};
    profiles_[2] = {3, "困难",  6, 2000, 0.00, 0.8, 0.7, "无失误，深度6"};
    profiles_[3] = {4, "专家",  8, 3000, 0.00, 1.0, 1.0, "最强，深度8"};
    profile_count_ = 4;
}

// 极简 JSON 解析：逐行读取，提取字段值。
// 只处理 difficulty_config.json 的固定结构，不是通用解析器。
static std::string extractStringValue(const std::string& line, const std::string& key) {
    // 格式: "key": "value"
    auto kpos = line.find('"' + key + '"');
    if (kpos == std::string::npos) return {};
    auto colon = line.find(':', kpos);
    if (colon == std::string::npos) return {};
    auto q1 = line.find('"', colon + 1);
    if (q1 == std::string::npos) return {};
    auto q2 = line.find('"', q1 + 1);
    if (q2 == std::string::npos) return {};
    return line.substr(q1 + 1, q2 - q1 - 1);
}

static bool extractNumberValue(const std::string& line, const std::string& key, double& out) {
    // 格式: "key": number
    auto kpos = line.find('"' + key + '"');
    if (kpos == std::string::npos) return false;
    auto colon = line.find(':', kpos);
    if (colon == std::string::npos) return false;
    // 跳过空格
    std::size_t vpos = colon + 1;
    while (vpos < line.size() && (line[vpos] == ' ' || line[vpos] == '\t')) ++vpos;
    try {
        std::size_t consumed = 0;
        out = std::stod(line.substr(vpos), &consumed);
        return consumed > 0;
    } catch (...) {
        return false;
    }
}

bool DifficultyConfig::loadFromFile(const std::string& file_path) {
    std::ifstream f(file_path);
    if (!f.is_open()) return false;

    // 临时存储解析中的配置项
    DifficultyProfile tmp_profiles[MAX_PROFILES];
    int count = 0;
    DifficultyProfile cur{};
    bool in_item = false;

    std::string line;
    while (std::getline(f, line) && count < MAX_PROFILES) {
        // 检测对象开始
        if (line.find('{') != std::string::npos && line.find('"') != std::string::npos) {
            in_item = true;
            cur = DifficultyProfile{};
        }

        if (!in_item) continue;

        double dval = 0.0;
        std::string sval;

        if (extractNumberValue(line, "id", dval))             cur.id             = static_cast<int>(dval);
        if (extractNumberValue(line, "search_depth", dval))   cur.search_depth   = static_cast<int>(dval);
        if (extractNumberValue(line, "time_limit_ms", dval))  cur.time_limit_ms  = static_cast<int>(dval);
        if (extractNumberValue(line, "error_rate", dval))     cur.error_rate     = dval;
        if (extractNumberValue(line, "attack_weight", dval))  cur.attack_weight  = dval;
        if (extractNumberValue(line, "defend_weight", dval))  cur.defend_weight  = dval;

        sval = extractStringValue(line, "name");
        if (!sval.empty()) cur.name = sval;

        sval = extractStringValue(line, "description");
        if (!sval.empty()) cur.description = sval;

        // 检测对象结束
        if (line.find('}') != std::string::npos && in_item && cur.id > 0) {
            tmp_profiles[count++] = cur;
            in_item = false;
        }
    }

    if (count == 0) return false; // 解析失败，保留默认配置

    for (int i = 0; i < count; ++i) profiles_[i] = tmp_profiles[i];
    profile_count_ = count;
    return true;
}

const DifficultyProfile& DifficultyConfig::getProfile(int difficulty_id) const {
    for (int i = 0; i < profile_count_; ++i) {
        if (profiles_[i].id == difficulty_id) return profiles_[i];
    }
    // 找不到时返回普通难度（id=2）
    for (int i = 0; i < profile_count_; ++i) {
        if (profiles_[i].id == default_id_) return profiles_[i];
    }
    return profiles_[0];
}

int DifficultyConfig::profileCount() const {
    return profile_count_;
}

} // namespace game_core
