#include "humanix/command.h"
#include "humanix/dispatcher.h"
#include "humanix/utils.h"
#include <fstream>
#include <iostream>
#include <algorithm>

namespace humanix {

class GrepCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "grep"; }
    
    [[nodiscard]] std::string description() const override {
        return "Search for patterns in files or input";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "grep [i] [n] [r] <pattern> [file...]\n"
               "\nOptions:\n"
               "  [i]      Case insensitive\n"
               "  [n]      Show line numbers\n"
               "  [r]      Recursive search in directories\n"
               "\nExamples:\n"
               "  grep \"hello\" file.txt          # Search in file\n"
               "  grep [i] [n] \"error\" *.log      # Case insensitive with line numbers\n"
               "  grep [r] \"main\" src/            # Recursive search";
    }
    
    CommandResult execute(const std::vector<std::string>& args) override {
        CommandResult result;
        
        if (args.empty()) {
            result.exit_code = 1;
            result.error = "Error: Missing pattern.\n";
            result.error += "Usage: " + usage();
            return result;
        }
        
        // 解析选项
        bool case_insensitive = utils::has_option(args, "i");
        bool show_line_numbers = utils::has_option(args, "n");
        bool recursive = utils::has_option(args, "r");
        
        // 获取模式和其他参数
        auto positional = utils::filter_positional_args(args);
        if (positional.empty()) {
            result.exit_code = 1;
            result.error = "Error: Missing pattern.";
            return result;
        }
        
        std::string pattern = positional[0];
        std::vector<std::string> files;
        for (size_t i = 1; i < positional.size(); i++) {
            files.push_back(positional[i]);
        }
        
        try {
            // 如果没有指定文件，从标准输入读取（暂不支持）
            if (files.empty()) {
                result.error = "Error: No files specified. Reading from stdin not supported yet.";
                result.exit_code = 1;
                return result;
            }
            
            int total_matches = 0;
            
            for (const auto& filepath : files) {
                if (!utils::path_exists(filepath)) {
                    result.error += "grep: " + filepath + ": No such file or directory\n";
                    continue;
                }
                
                if (utils::is_directory(filepath) && recursive) {
                    // 递归搜索目录
                    for (const auto& entry : std::filesystem::recursive_directory_iterator(filepath)) {
                        if (entry.is_regular_file()) {
                            total_matches += search_file(entry.path().string(), pattern, 
                                                       case_insensitive, show_line_numbers, result);
                        }
                    }
                } else if (utils::is_file(filepath)) {
                    total_matches += search_file(filepath, pattern, case_insensitive, 
                                               show_line_numbers, result);
                }
            }
            
            if (total_matches == 0) {
                result.exit_code = 1; // grep 惯例：没找到返回 1
            }
            
        } catch (const std::exception& e) {
            result.exit_code = 2;
            result.error = "Error: " + std::string(e.what());
        }
        
        return result;
    }
    
private:
    int search_file(const std::string& filepath, const std::string& pattern,
                   bool case_insensitive, bool show_line_numbers, CommandResult& result) {
        int matches = 0;
        
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                result.error += "grep: " + filepath + ": Cannot open file\n";
                return 0;
            }
            
            std::string line;
            int line_num = 0;
            while (std::getline(file, line)) {
                line_num++;
                
                std::string search_line = line;
                std::string search_pattern = pattern;
                
                if (case_insensitive) {
                    // 转换为小写
                    std::transform(search_line.begin(), search_line.end(), 
                                 search_line.begin(), ::tolower);
                    std::transform(search_pattern.begin(), search_pattern.end(), 
                                 search_pattern.begin(), ::tolower);
                }
                
                if (search_line.find(search_pattern) != std::string::npos) {
                    matches++;
                    
                    if (show_line_numbers) {
                        result.output += filepath + ":" + std::to_string(line_num) + ":" + line + "\n";
                    } else {
                        result.output += filepath + ":" + line + "\n";
                    }
                }
            }
            
        } catch (const std::exception& e) {
            result.error += "grep: " + filepath + ": " + e.what() + "\n";
        }
        
        return matches;
    }
};

static struct GrepRegistrar {
    GrepRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<GrepCommand>());
    }
} grep_registrar;

} // namespace humanix
