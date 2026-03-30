#pragma once

#include <string>

// 难度配置值对象：存储单个难度级别的所有 AI 行为参数。
// 可从 JSON 文件加载，无需重新编译即可调整。
namespace game_core {

// 单个难度配置
struct DifficultyProfile {
    int    id;             // 难度 ID（1=简单, 2=普通, 3=困难, 4=专家）
    std::string name;      // 难度名称
    int    search_depth;   // 搜索深度上限
    int    time_limit_ms;  // 时间限制（毫秒）
    double error_rate;     // 错误率：走随机步的概率 [0.0, 1.0]
    double attack_weight;  // 进攻权重（影响评分时进攻棋型的分值倍率）
    double defend_weight;  // 防守权重（影响评分时防守棋型的分值倍率）
    std::string description; // 描述
};

// 难度配置加载器：从 JSON 文件读取所有难度配置。
// JSON 格式见 resources/difficulty_config.json。
class DifficultyConfig {
public:
    // 默认构造：加载内置默认配置（不依赖文件）
    DifficultyConfig();

    // 从 JSON 文件加载难度配置
    // file_path: JSON 文件路径
    // 返回是否加载成功；失败时保留默认配置
    bool loadFromFile(const std::string& file_path);

    // 根据难度 ID 获取配置（不存在则返回默认中等难度）
    const DifficultyProfile& getProfile(int difficulty_id) const;

    // 获取所有难度配置数量
    int profileCount() const;

private:
    // 初始化内置默认配置
    void initDefaults();

    static constexpr int MAX_PROFILES = 8;
    DifficultyProfile profiles_[MAX_PROFILES]; // 配置数组
    int profile_count_;                         // 实际配置数量
    int default_id_;                            // 默认难度 ID
};

} // namespace game_core
