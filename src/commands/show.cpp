#include "humanix/command.h"
#include "humanix/dispatcher.h"
#include "humanix/utils.h"
#include <fstream>
#include <sstream>

namespace humanix {

class ShowCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "show"; }
    
    [[nodiscard]] std::string description() const override {
        return "Display file contents";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "show [page] [follow] <file>\n"
               "\nOptions:\n"
               "  [page]   Page mode (show 20 lines at a time)\n"
               "  [follow] Follow mode (like tail -f, not implemented yet)\n"
               "\nExamples:\n"
               "  show README.md              # Show file\n"
               "  show [page] large.log       # Page through file";
    }
    
    CommandResult execute(const std::vector<std::string>& args) override {
        CommandResult result;
        
        // 解析选项
        bool page_mode = utils::has_option(args, "page");
        bool follow_mode = utils::has_option(args, "follow");
        
        if (follow_mode) {
            result.error = "Warning: [follow] mode not implemented yet.";
        }
        
        // 获取文件参数
        auto positional = utils::filter_positional_args(args);
        if (positional.empty()) {
            result.exit_code = 1;
            result.error = "Error: No file specified.\n";
            result.error += "Usage: " + usage();
            return result;
        }
        
        const std::string& filepath = positional[0];
        
        try {
            // 检查文件是否存在
            if (!utils::path_exists(filepath)) {
                result.exit_code = 1;
                result.error = "Error: File '" + filepath + "' does not exist.";
                return result;
            }
            
            if (!utils::is_file(filepath)) {
                result.exit_code = 1;
                result.error = "Error: '" + filepath + "' is not a regular file.";
                return result;
            }
            
            // 读取文件
            std::ifstream file(filepath);
            if (!file.is_open()) {
                result.exit_code = 2;
                result.error = "Error: Cannot open file '" + filepath + "'.";
                return result;
            }
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            
            std::string content = buffer.str();
            
            if (page_mode) {
                // 分页模式：每 20 行暂停
                std::istringstream stream(content);
                std::string line;
                int line_count = 0;
                
                while (std::getline(stream, line)) {
                    result.output += line + "\n";
                    line_count++;
                    
                    if (line_count % 20 == 0 && !stream.eof()) {
                        result.output += "-- More -- (Press Enter to continue)\n";
                        // 注意：在交互式模式下需要等待用户输入
                        // 这里简化处理，直接继续
                    }
                }
            } else {
                result.output = content;
            }
            
        } catch (const std::exception& e) {
            result.exit_code = 3;
            result.error = "Error: " + std::string(e.what());
        }
        
        return result;
    }
};

static struct ShowRegistrar {
    ShowRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<ShowCommand>());
    }
} show_registrar;

} // namespace humanix