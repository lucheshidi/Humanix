#include "humanix/command.h"
#include "humanix/dispatcher.h"
#include "humanix/utils.h"
#include <filesystem>
#include <iostream>

namespace humanix {

class DeleteCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "delete"; }
    
    [[nodiscard]] std::string description() const override {
        return "Delete files or directories (safe by default)";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "delete [r] [force] [preview] <target>...\n"
               "\nOptions:\n"
               "  [r]        Recursive delete (for directories)\n"
               "  [force]    Skip confirmation prompt\n"
               "  [preview]  Show what will be deleted without deleting\n"
               "\nExamples:\n"
               "  delete file.txt            # Delete a file\n"
               "  delete [r] mydir           # Delete directory recursively\n"
               "  delete [r] [preview] temp  # Preview before deletion\n"
               "  delete [r] [force] old     # Force delete without prompt";
    }
    
    CommandResult execute(const std::vector<std::string>& args) override {
        CommandResult result;
        
        // 解析选项
        const bool recursive = utils::has_option(args, "r");
        const bool force = utils::has_option(args, "force");
        const bool preview = utils::has_option(args, "preview");
        
        // 获取目标参数
        const auto positional = utils::filter_positional_args(args);
        if (positional.empty()) {
            result.exit_code = 1;
            result.error = "Error: No target specified.\n";
            result.error += "Usage: " + usage();
            return result;
        }
        
        try {
            for (const auto& target : positional) {
                if (!utils::path_exists(target)) {
                    result.error += "Warning: '" + target + "' does not exist. Skipped.\n";
                    continue;
                }
                
                bool is_dir = utils::is_directory(target);
                
                // 如果是目录但没有 -r 选项
                if (is_dir && !recursive) {
                    result.error += "Error: '" + target + "' is a directory. Use [r] to delete recursively.\n";
                    result.exit_code = 1;
                    continue;
                }
                
                // 预览模式
                if (preview) {
                    result.output += "Would delete: " + target;
                    if (is_dir) {
                        // 统计目录内容
                        size_t file_count = 0, dir_count = 0;
                        for (const auto& entry : std::filesystem::recursive_directory_iterator(target)) {
                            if (entry.is_directory()) dir_count++;
                            else file_count++;
                        }
                        result.output += " (directory with " + std::to_string(file_count) + 
                                       " files and " + std::to_string(dir_count) + " subdirectories)";
                    }
                    result.output += "\n";
                    continue;
                }
                
                // 确认提示（除非 force）
                if (!force) {
                    std::cout << "Delete '" << target << "'? (y/N): ";
                    char response;
                    std::cin >> response;
                    if (response != 'y' && response != 'Y') {
                        result.output += "Skipped: " + target + "\n";
                        continue;
                    }
                }
                
                // 执行删除
                if (is_dir && recursive) {
                    std::filesystem::remove_all(target);
                    result.output += "Deleted directory: " + target + "\n";
                } else {
                    std::filesystem::remove(target);
                    result.output += "Deleted: " + target + "\n";
                }
            }
            
            if (result.error.empty() && result.output.empty()) {
                result.output = "Nothing to delete.\n";
            }
            
        } catch (const std::filesystem::filesystem_error& e) {
            result.exit_code = 2;
            result.error = "Filesystem error: " + std::string(e.what());
        } catch (const std::exception& e) {
            result.exit_code = 3;
            result.error = "Error: " + std::string(e.what());
        }
        
        return result;
    }
};

static struct DeleteRegistrar {
    DeleteRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<DeleteCommand>());
    }
} delete_registrar;

} // namespace humanix