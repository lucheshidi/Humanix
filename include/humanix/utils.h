#ifndef HUMANIX_UTILS_H
#define HUMANIX_UTILS_H

#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace humanix {
namespace utils {

/**
 * 检查字符串是否为选项标志（以 - 或 [ 开头）
 */
inline bool is_option(const std::string& arg) {
    return !arg.empty() && (arg[0] == '-' || arg[0] == '[');
}

/**
 * 从参数列表中提取选项
 * @param args 原始参数列表
 * @param option_name 选项名称（如 "r", "force"）
 * @return 是否找到该选项
 */
inline bool has_option(const std::vector<std::string>& args, const std::string& option_name) {
    for (const auto& arg : args) {
        if (arg == "-" + option_name || 
            arg == "--" + option_name ||
            arg == "[" + option_name + "]") {
            return true;
        }
    }
    return false;
}

/**
 * 获取选项的值（支持 key=value 格式）
 * @param args 原始参数列表
 * @param key 键名
 * @return 选项值，如果不存在返回空字符串
 */
inline std::string get_option_value(const std::vector<std::string>& args, const std::string& key) {
    for (const auto& arg : args) {
        // 支持 key=value 格式
        if (arg.find(key + "=") == 0) {
            return arg.substr(key.length() + 1);
        }
    }
    return "";
}

/**
 * 过滤掉所有选项，只保留位置参数
 */
inline std::vector<std::string> filter_positional_args(const std::vector<std::string>& args) {
    std::vector<std::string> positional;
    for (const auto& arg : args) {
        if (!is_option(arg) && arg.find('=') == std::string::npos) {
            positional.push_back(arg);
        }
    }
    return positional;
}

/**
 * 创建目录（包括中间目录）
 */
inline bool create_directories_recursive(const std::filesystem::path& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::filesystem::filesystem_error& e) {
        return false;
    }
}

/**
 * 检查路径是否存在
 */
inline bool path_exists(const std::string& path) {
    return std::filesystem::exists(path);
}

/**
 * 判断是否是目录
 */
inline bool is_directory(const std::string& path) {
    return std::filesystem::is_directory(path);
}

/**
 * 判断是否是文件
 */
inline bool is_file(const std::string& path) {
    return std::filesystem::is_regular_file(path);
}

/**
 * 获取文件大小（字节）
 */
inline uintmax_t get_file_size(const std::string& path) {
    try {
        return std::filesystem::file_size(path);
    } catch (...) {
        return 0;
    }
}

/**
 * 格式化文件大小为人类可读格式
 */
inline std::string format_file_size(uintmax_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unit_index < 4) {
        size /= 1024.0;
        unit_index++;
    }
    
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[unit_index]);
    return std::string(buffer);
}

} // namespace utils
} // namespace humanix

#endif // HUMANIX_UTILS_H
