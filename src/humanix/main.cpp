#include "humanix/dispatcher.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>

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

int main() {
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
        char* input = nullptr;
#ifdef HAVE_READLINE
        input = readline(prompt.c_str());
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