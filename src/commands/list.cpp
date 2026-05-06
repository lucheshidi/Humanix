#include "humanix/command.h"
#include "humanix/dispatcher.h"
#include "humanix/utils.h"
#include <filesystem>
#include <iomanip>
#include <chrono>
#include <sstream>

namespace humanix {

class ListCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "list"; }
    
    [[nodiscard]] std::string description() const override {
        return "List directory contents with various display options";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "list [human] [sort=name|size|time] [long] <path>\n"
               "\nOptions:\n"
               "  [human]      Human-readable file sizes\n"
               "  [sort=name]  Sort by name (default)\n"
               "  [sort=size]  Sort by size\n"
               "  [sort=time]  Sort by modification time\n"
               "  [long]       Long format (detailed info)\n"
               "\nExamples:\n"
               "  list                     # Current directory\n"
               "  list /home/user          # Specific path\n"
               "  list [human] [long]      # Detailed with human sizes\n"
               "  list [sort=size] [long]  # Sorted by size";
    }
    
    CommandResult execute(const std::vector<std::string>& args) override {
        CommandResult result;
        
        // 解析选项
        bool human_readable = utils::has_option(args, "human");
        bool long_format = utils::has_option(args, "long");
        
        std::string sort_by = utils::get_option_value(args, "sort");
        if (sort_by.empty()) {
            sort_by = "name"; // 默认按名称排序
        }
        
        // 获取路径参数
        auto positional = utils::filter_positional_args(args);
        std::string path = positional.empty() ? "." : positional[0];
        
        try {
            if (!utils::path_exists(path)) {
                result.exit_code = 1;
                result.error = "Error: Path '" + path + "' does not exist.";
                return result;
            }
            
            if (!utils::is_directory(path)) {
                result.exit_code = 1;
                result.error = "Error: '" + path + "' is not a directory.";
                return result;
            }
            
            // 收集目录内容
            struct EntryInfo {
                std::string name;
                bool is_dir{};
                uintmax_t size{};
                std::filesystem::file_time_type mod_time;
            };
            
            std::vector<EntryInfo> entries;
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                EntryInfo info;
                info.name = entry.path().filename().string();
                info.is_dir = entry.is_directory();
                info.size = entry.is_regular_file() ? entry.file_size() : 0;
                info.mod_time = entry.last_write_time();
                entries.push_back(info);
            }
            
            // 排序
            if (sort_by == "name") {
                std::sort(entries.begin(), entries.end(), 
                    [](const EntryInfo& a, const EntryInfo& b) {
                        return a.name < b.name;
                    });
            } else if (sort_by == "size") {
                std::sort(entries.begin(), entries.end(), 
                    [](const EntryInfo& a, const EntryInfo& b) {
                        return a.size > b.size; // 大到小
                    });
            } else if (sort_by == "time") {
                std::sort(entries.begin(), entries.end(), 
                    [](const EntryInfo& a, const EntryInfo& b) {
                        return a.mod_time > b.mod_time; // 新到旧
                    });
            }
            
            // 输出
            std::ostringstream oss;
            
            if (long_format) {
                // 长格式输出
                oss << "Total: " << entries.size() << " items\n\n";
                
                for (const auto& entry : entries) {
                    // 类型标识
                    oss << (entry.is_dir ? "D " : "F ");
                    
                    // 大小
                    if (human_readable) {
                        oss << std::setw(10) << utils::format_file_size(entry.size);
                    } else {
                        oss << std::setw(12) << entry.size;
                    }
                    
                    // 修改时间（简化版，只输出日期）
                    auto time_t_mod = std::chrono::system_clock::to_time_t(
                        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                            entry.mod_time - std::filesystem::file_time_type::clock::now() +
                            std::chrono::system_clock::now()
                        )
                    );
                    char time_buf[64];
                    std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M", std::localtime(&time_t_mod));
                    oss << "  " << time_buf;
                    
                    // 名称
                    oss << "  " << entry.name << "\n";
                }
            } else {
                // 简洁格式输出（类似 ls）
                for (const auto& entry : entries) {
                    if (entry.is_dir) {
                        oss << "[" << entry.name << "]  ";
                    } else {
                        oss << entry.name << "  ";
                    }
                }
                if (!entries.empty()) {
                    oss << "\n";
                }
            }
            
            result.output = oss.str();
            
        } catch (const std::filesystem::filesystem_error& e) {
            result.exit_code = 2;
            result.error = std::string("Filesystem error: ") + e.what();
        } catch (const std::exception& e) {
            result.exit_code = 3;
            result.error = std::string("Error: ") + e.what();
        }
        
        return result;
    }
};

static struct ListRegistrar {
    ListRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<ListCommand>());
    }
} list_registrar;

} // namespace humanix