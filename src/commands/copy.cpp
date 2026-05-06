#include "humanix/command.h"
#include "humanix/dispatcher.h"
#include "humanix/utils.h"
#include <filesystem>

namespace humanix {

class CopyCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "copy"; }
    
    [[nodiscard]] std::string description() const override {
        return "Copy files or directories";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "copy [r] [force] <src> <dst>\n"
               "\nOptions:\n"
               "  [r]      Recursive copy (for directories)\n"
               "  [force]  Overwrite destination without prompt\n"
               "\nExamples:\n"
               "  copy file.txt backup.txt           # Copy file\n"
               "  copy [r] src/ backup/              # Copy directory recursively\n"
               "  copy [r] [force] old/ new/         # Force overwrite";
    }
    
    CommandResult execute(const std::vector<std::string>& args) override {
        CommandResult result;
        
        // 解析选项
        const bool recursive = utils::has_option(args, "r");
        const bool force = utils::has_option(args, "force");
        
        // 获取源和目标
        const auto positional = utils::filter_positional_args(args);
        if (positional.size() < 2) {
            result.exit_code = 1;
            result.error = "Error: Missing source or destination.\n";
            result.error += "Usage: " + usage();
            return result;
        }
        
        const std::string& src = positional[0];
        const std::string& dst = positional[1];
        
        try {
            // 检查源是否存在
            if (!utils::path_exists(src)) {
                result.exit_code = 1;
                result.error = "Error: Source '" + src + "' does not exist.";
                return result;
            }

            const bool src_is_dir = utils::is_directory(src);
            
            // 如果是目录但没有 -r 选项
            if (src_is_dir && !recursive) {
                result.exit_code = 1;
                result.error = "Error: '" + src + "' is a directory. Use [r] to copy recursively.";
                return result;
            }
            
            // 检查目标是否已存在
            if (utils::path_exists(dst) && !force) {
                result.exit_code = 1;
                result.error = "Error: Destination '" + dst + "' already exists. Use [force] to overwrite.";
                return result;
            }
            
            // 执行复制
            if (src_is_dir && recursive) {
                std::filesystem::copy(src, dst, 
                    std::filesystem::copy_options::recursive | 
                    std::filesystem::copy_options::overwrite_existing);
                result.output = "Directory copied: " + src + " -> " + dst + "\n";
            } else {
                std::filesystem::copy_file(src, dst, 
                    std::filesystem::copy_options::overwrite_existing);
                result.output = "File copied: " + src + " -> " + dst + "\n";
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

static struct CopyRegistrar {
    CopyRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<CopyCommand>());
    }
} copy_registrar;

} // namespace humanix