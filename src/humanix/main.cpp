#include "humanix/dispatcher.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>

#ifdef _WIN32
#include <io.h>
#define access _access
#else
#include <unistd.h>
#endif

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

using namespace humanix;

// ANSI 颜色代码
namespace colors {
    const char* RESET = "\033[0m";
    const char* BOLD = "\033[1m";
    const char* GREEN = "\033[32m";
    const char* BLUE = "\033[34m";
    const char* YELLOW = "\033[33m";
    const char* RED = "\033[31m";
    const char* CYAN = "\033[36m";
}

/**
 * 显示用法说明
 */
void show_usage() {
    std::cout << "Humanix v1.0 - Modern Command-Line Tools\n\n";
    std::cout << "Usage:\n";
    std::cout << "  humanix [shell]              # Enter interactive shell\n";
    std::cout << "  humanix [gl]                 # Generate command symlinks\n";
    std::cout << "  humanix [help]               # Show this help\n\n";
    std::cout << "Examples:\n";
    std::cout << "  humanix shell                # Start shell\n";
    std::cout << "  humanix gl                   # Create symlinks for all commands\n";
    std::cout << "  ./crt test.txt               # Use command directly (after gl)\n\n";
    std::cout << "Available commands:\n";
    
    auto commands = Dispatcher::instance().list_commands();
    for (const auto& cmd : commands) {
        std::cout << "  " << cmd << "\n";
    }
}

/**
 * 生成命令包装脚本
 */
int generate_command_links(const std::string& target_dir = "") {
    const auto commands = Dispatcher::instance().list_commands();

    const std::string exe_path = std::filesystem::canonical("/proc/self/exe").string();
    
    // 确定目标目录
    std::string install_dir;
    if (!target_dir.empty()) {
        install_dir = target_dir;
    } else {
        // 默认：可执行文件所在目录
        install_dir = std::filesystem::path(exe_path).parent_path().string();
    }
    
    // 检查目录是否存在
    if (!std::filesystem::exists(install_dir)) {
        std::cerr << "Error: Directory '" << install_dir << "' does not exist." << std::endl;
        return 1;
    }
    
    // 检查是否有写入权限
    if (access(install_dir.c_str(), W_OK) != 0) {
        std::cerr << "Error: Cannot write to " << install_dir << std::endl;
        std::cerr << "Try running with sudo or specify a different directory." << std::endl;
        return 1;
    }
    
    std::cout << "Generating command wrappers in " << install_dir << "...\n";
    
    int created = 0;
#ifdef _WIN32
    // Windows: 生成 .bat 文件
    for (const auto& cmd : commands) {
        std::string script_path = install_dir + "/" + cmd + ".bat";
        
        std::ofstream script(script_path);
        if (!script.is_open()) {
            std::cerr << "  Failed to create " << cmd << ".bat\n";
            continue;
        }
        
        script << "@echo off\n";
        script << "\"" << exe_path << "\" %*\n";
        script.close();
        
        std::cout << "  Created: " << cmd << ".bat\n";
        created++;
    }
#else
    // Linux: 生成 shell 脚本
    for (const auto& cmd : commands) {
        std::string script_path = install_dir + "/" + cmd;
        
        std::ofstream script(script_path);
        if (!script.is_open()) {
            std::cerr << "  Failed to create " << cmd << "\n";
            continue;
        }
        
        script << "#!/bin/sh\n";
        script << "exec \"" << exe_path << "\" \"$@\"\n";
        script.close();
        
        // 设置可执行权限
        chmod(script_path.c_str(), 0755);
        
        std::cout << "  Created: " << cmd << "\n";
        created++;
    }
#endif
    
    std::cout << "\nGenerated " << created << " command wrappers.\n";
    std::cout << "You can now use commands directly: crt, list, delete, etc.\n";
    
    return 0;
}

/**
 * 获取当前工作目录的简短显示
 */
std::string get_current_dir_display() {
    try {
        auto path = std::filesystem::current_path();
        auto str = path.string();
        
        // 如果在项目目录下，显示相对路径
        auto home = std::filesystem::current_path();
        if (str.length() > 40) {
            // 只显示最后两级目录
            auto it = path.end();
            --it; // 文件名
            std::string last = it->string();
            if (it != path.begin()) {
                --it;
                std::string second_last = it->string();
                return ".../" + second_last + "/" + last;
            }
            return last;
        }
        return str;
    } catch (...) {
        return "?";
    }
}

/**
 * 获取自定义提示符（从环境变量 HUMANIX_PS1）
 * 支持占位符：
 *   %u - 用户名
 *   %h - 主机名
 *   %d - 当前目录
 *   %n - 换行
 */
std::string get_custom_prompt() {
    const char* ps1_env = std::getenv("HUMANIX_PS1");
    if (!ps1_env || std::string(ps1_env).empty()) {
        return ""; // 使用默认提示符
    }
    
    std::string ps1 = ps1_env;
    
    // 替换占位符
    std::string cwd = get_current_dir_display();
    
    size_t pos;
    while ((pos = ps1.find("%d")) != std::string::npos) {
        ps1.replace(pos, 2, cwd);
    }
    
    // 简单获取用户名（跨平台）
    const char* user = std::getenv("USERNAME");
    if (!user) user = std::getenv("USER");
    std::string username = user ? user : "user";
    
    while ((pos = ps1.find("%u")) != std::string::npos) {
        ps1.replace(pos, 2, username);
    }
    
    // 主机名（简化，只取 COMPUTERNAME）
    const char* host = std::getenv("COMPUTERNAME");
    if (!host) host = std::getenv("HOSTNAME");
    std::string hostname = host ? host : "host";
    
    while ((pos = ps1.find("%h")) != std::string::npos) {
        ps1.replace(pos, 2, hostname);
    }
    
    // 换行
    while ((pos = ps1.find("%n")) != std::string::npos) {
        ps1.replace(pos, 2, "\n");
    }
    
    return ps1;
}

/**
 * 解析命令行输入为命令名和参数
 */
std::pair<std::string, std::vector<std::string>> parse_input(const std::string& input) {
    std::istringstream iss(input);
    std::string command;
    std::vector<std::string> args;
    
    if (iss >> command) {
        std::string arg;
        while (iss >> arg) {
            args.push_back(arg);
        }
    }
    
    return {command, args};
}

int main(int argc, char* argv[]) {
    // 解析命令行参数
    bool shell_mode = false;
    bool generate_links = false;
    std::string links_dir; // 符号链接目标目录
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "shell" || arg == "[shell]") {
            shell_mode = true;
        } else if (arg == "gl" || arg == "[gl]" || arg == "generate-links" || arg == "[generate-links]") {
            generate_links = true;
            // 检查下一个参数是否为目录
            if (i + 1 < argc && argv[i + 1][0] != '[' && argv[i + 1][0] != '-') {
                links_dir = argv[++i];
            }
        } else if (arg == "help" || arg == "--help" || arg == "-h") {
            show_usage();
            return 0;
        }
    }
    
    // 没有参数，显示用法
    if (!shell_mode && !generate_links) {
        show_usage();
        return 1;
    }
    
    // 生成符号链接模式
    if (generate_links) {
        return generate_command_links(links_dir);
    }
    
    // Shell 模式
    std::cout << colors::BOLD << colors::CYAN;
    std::cout << "Humanix v1.0" << colors::RESET << "\n\n";
    
    std::string line;
    while (true) {
        // 显示提示符
        std::string custom_ps1 = get_custom_prompt();
        std::string prompt;
        
        if (!custom_ps1.empty()) {
            prompt = custom_ps1 + " ";
        } else {
            std::string cwd = get_current_dir_display();
            prompt = "humanix [" + cwd + "] > ";
        }
        
        // 读取输入（支持 readline）
#ifdef HAVE_READLINE
        char* input = readline(prompt.c_str());
        if (!input) break; // EOF
        
        // 添加到历史
        if (*input) {
            add_history(input);
            line = std::string(input);
        }
        free(input);
#else
        // 简单模式：直接输出提示符
        std::cout << colors::GREEN << "humanix" << colors::RESET
                  << colors::BLUE << " [" << get_current_dir_display() << "]" << colors::RESET
                  << " " << colors::YELLOW << ">" << colors::RESET << " ";
        std::flush(std::cout);
        
        if (!std::getline(std::cin, line)) {
            break; // EOF
        }
#endif
        
        // 跳过空行
        if (line.empty()) {
            continue;
        }
        
        auto [command_name, args] = parse_input(line);
        
        // 内置命令：exit 和 help
        if (command_name == "exit") {
            std::cout << colors::CYAN << "Goodbye!" << colors::RESET << "\n";
            break;
        } else if (command_name == "help") {
            std::string help_cmd = args.empty() ? "" : args[0];
            std::string help_text = Dispatcher::instance().get_help(help_cmd);
            // 简单高亮帮助文本
            std::cout << colors::CYAN << help_text << colors::RESET;
            continue;
        }
        
        // 分发命令
        auto result = Dispatcher::instance().dispatch(command_name, args);
        
        if (!result.output.empty()) {
            std::cout << result.output;
        }
        if (!result.error.empty()) {
            std::cerr << colors::RED << result.error << colors::RESET;
        }
        
        if (result.exit_code != 0) {
            std::cout << colors::RED << "\n[ERROR] Command failed (exit code: " << result.exit_code << ")" << colors::RESET << "\n";
        } else if (!result.output.empty()) {
            std::cout << colors::GREEN << "[OK]" << colors::RESET << "\n";
        }
    }
    
    return 0;
}