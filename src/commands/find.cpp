#include "humanix/command.h"
#include "humanix/dispatcher.h"
#include "humanix/utils.h"
#include <filesystem>
#include <fstream>

namespace humanix {

class FindCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "find"; }
    
    [[nodiscard]] std::string description() const override {
        return "Search for files by name or content";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "find [name=*.log] [content=keyword] [path=.]\n"
               "\nOptions:\n"
               "  [name=pattern]   Search by filename pattern (supports wildcards)\n"
               "  [content=text]   Search by file content\n"
               "  [path=dir]       Search path (default: current directory)\n"
               "\nExamples:\n"
               "  find [name=*.txt]                # Find all .txt files\n"
               "  find [content=hello]             # Find files containing 'hello'\n"
               "  find [name=*.cpp] [path=src/]    # Find .cpp in src/";
    }
    
    CommandResult execute(const std::vector<std::string>& args) override {
        CommandResult result;
        
        // 解析选项
        const std::string name_pattern = utils::get_option_value(args, "name");
        const std::string content_search = utils::get_option_value(args, "content");
        std::string search_path = utils::get_option_value(args, "path");
        if (search_path.empty()) {
            search_path = ".";
        }
        
        if (name_pattern.empty() && content_search.empty()) {
            result.exit_code = 1;
            result.error = "Error: Must specify [name=...] or [content=...].\n";
            result.error += "Usage: " + usage();
            return result;
        }
        
        try {
            if (!utils::path_exists(search_path)) {
                result.exit_code = 1;
                result.error = "Error: Path '" + search_path + "' does not exist.";
                return result;
            }
            
            int found_count = 0;
            
            for (const auto& entry : std::filesystem::recursive_directory_iterator(search_path)) {
                if (!entry.is_regular_file()) continue;
                
                std::string filename = entry.path().filename().string();
                std::string filepath = entry.path().string();
                
                bool match = true;
                
                // 名称匹配（简单通配符支持）
                if (!name_pattern.empty()) {
                    if (!wildcard_match(filename, name_pattern)) {
                        match = false;
                    }
                }
                
                // 内容匹配
                if (match && !content_search.empty()) {
                    if (!file_contains(filepath, content_search)) {
                        match = false;
                    }
                }
                
                if (match) {
                    result.output += filepath + "\n";
                    found_count++;
                }
            }
            
            if (found_count == 0) {
                result.output = "No matches found.\n";
            } else {
                result.output += "\nFound " + std::to_string(found_count) + " file(s).\n";
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
    
private:
    // 简单通配符匹配（支持 * 和 ?）
    static bool wildcard_match(const std::string& str, const std::string& pattern) {
        size_t s = 0, p = 0, star = std::string::npos, ss = 0;
        
        while (s < str.size()) {
            if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == str[s])) {
                s++; p++;
            } else if (p < pattern.size() && pattern[p] == '*') {
                star = p++; ss = s;
            } else if (star != std::string::npos) {
                p = star + 1; s = ++ss;
            } else {
                return false;
            }
        }
        
        while (p < pattern.size() && pattern[p] == '*') p++;
        return p == pattern.size();
    }
    
    // 检查文件是否包含指定文本
    static bool file_contains(const std::string& filepath, const std::string& text) {
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) return false;
            
            std::string line;
            while (std::getline(file, line)) {
                if (line.find(text) != std::string::npos) {
                    return true;
                }
            }
            return false;
        } catch (...) {
            return false;
        }
    }
};

static struct FindRegistrar {
    FindRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<FindCommand>());
    }
} find_registrar;

} // namespace humanix