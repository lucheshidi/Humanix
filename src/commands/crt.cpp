#include "humanix/command.h"
#include "humanix/dispatcher.h"
#include "humanix/utils.h"
#include <filesystem>
#include <fstream>

namespace humanix {

class CrtCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "crt"; }
    
    [[nodiscard]] std::string description() const override {
        return "Create file or directory (smart mode by default)";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "crt [d] [f] [force] <name>\n"
               "\nOptions:\n"
               "  [d]      Force create directory (recursive)\n"
               "  [f]      Force create file\n"
               "  [force]  Overwrite if exists\n"
               "\nExamples:\n"
               "  crt myproject          # Smart: dir (no extension)\n"
               "  crt file.txt           # Smart: file (has extension)\n"
               "  crt [d] src/utils      # Create directory\n"
               "  crt [f] main.cpp       # Create file";
    }
    
    CommandResult execute(const std::vector<std::string>& args) override {
        CommandResult result;
        
        // 检查参数
        auto positional = utils::filter_positional_args(args);
        if (positional.empty()) {
            result.exit_code = 1;
            result.error = "Error: Missing target name.\n";
            result.error += "Usage: " + usage();
            return result;
        }
        
        // 检查冲突选项
        bool force_dir = utils::has_option(args, "d");
        bool force_file = utils::has_option(args, "f");
        bool force_overwrite = utils::has_option(args, "force");
        
        if (force_dir && force_file) {
            result.exit_code = 1;
            result.error = "Error: Cannot use both [d] and [f] options.";
            return result;
        }
        
        const std::string& target = positional[0];
        
        // 智能模式判断
        bool should_be_file = false;
        if (!force_dir && !force_file) {
            // 有常见文件扩展名 → 文件，否则 → 目录
            std::filesystem::path p(target);
            std::string ext = p.extension().string();
            if (!ext.empty()) {
                should_be_file = true;
            }
        } else if (force_file) {
            should_be_file = true;
        }
        
        try {
            if (should_be_file) {
                // 创建文件
                if (utils::path_exists(target) && !force_overwrite) {
                    result.exit_code = 1;
                    result.error = "Error: File '" + target + "' already exists. Use [force] to overwrite.";
                    return result;
                }
                
                std::ofstream file(target);
                if (!file.is_open()) {
                    result.exit_code = 2;
                    result.error = "Error: Failed to create file '" + target + "'.";
                    return result;
                }
                file.close();
                result.output = "File created: " + target + "\n";
                
            } else {
                // 创建目录（递归）
                if (utils::path_exists(target) && !force_overwrite) {
                    result.exit_code = 1;
                    result.error = "Error: Directory '" + target + "' already exists. Use [force] to overwrite.";
                    return result;
                }
                
                if (!utils::create_directories_recursive(target)) {
                    result.exit_code = 2;
                    result.error = "Error: Failed to create directory '" + target + "'.";
                    return result;
                }
                result.output = "Directory created: " + target + "\n";
            }
            
        } catch (const std::exception& e) {
            result.exit_code = 3;
            result.error = std::string("Error: ") + e.what();
        }
        
        return result;
    }
};

// 自动注册命令（在程序启动时）
static struct CrtRegistrar {
    CrtRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<CrtCommand>());
    }
} crt_registrar;

} // namespace humanix