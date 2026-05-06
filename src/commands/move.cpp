#include "humanix/command.h"
#include "humanix/dispatcher.h"
#include "humanix/utils.h"
#include <filesystem>

namespace humanix {

class MoveCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "move"; }
    
    [[nodiscard]] std::string description() const override {
        return "Move or rename files and directories";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "move [force] <src> <dst>\n"
               "\nOptions:\n"
               "  [force]  Overwrite destination without prompt\n"
               "\nExamples:\n"
               "  move old.txt new.txt           # Rename file\n"
               "  move file.txt /tmp/            # Move to directory\n"
               "  move [force] src/ dest/        # Force move";
    }
    
    CommandResult execute(const std::vector<std::string>& args) override {
        CommandResult result;
        
        // 解析选项
        bool force = utils::has_option(args, "force");
        
        // 获取源和目标
        auto positional = utils::filter_positional_args(args);
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
            
            // 检查目标是否已存在
            if (utils::path_exists(dst) && !force) {
                result.exit_code = 1;
                result.error = "Error: Destination '" + dst + "' already exists. Use [force] to overwrite.";
                return result;
            }
            
            // 如果目标是目录，将源移动到该目录下
            if (utils::is_directory(dst)) {
                std::filesystem::path dst_path = std::filesystem::path(dst) / std::filesystem::path(src).filename();
                std::filesystem::rename(src, dst_path);
                result.output = "Moved: " + src + " -> " + dst_path.string() + "\n";
            } else {
                // 直接重命名/移动
                std::filesystem::rename(src, dst);
                result.output = "Moved: " + src + " -> " + dst + "\n";
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

static struct MoveRegistrar {
    MoveRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<MoveCommand>());
    }
} move_registrar;

} // namespace humanix