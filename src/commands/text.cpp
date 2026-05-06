#include "humanix/command.h"
#include "humanix/dispatcher.h"
#include "humanix/utils.h"
#include <fstream>
#include <sstream>

namespace humanix {

class TextCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "echo"; }
    
    [[nodiscard]] std::string description() const override {
        return "Text processing and output (echo, grep, replace, count)";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "echo \"content\" [> file] [>> file]\n"
               "echo grep \"keyword\" <file>\n"
               "echo replace \"old\" \"new\" <file> [preview]\n"
               "echo count <file>\n"
               "\nExamples:\n"
               "  echo \"Hello World\"              # Print text\n"
               "  echo \"test\" > file.txt           # Write to file\n"
               "  echo \"test\" >> file.txt          # Append to file\n"
               "  echo grep \"hello\" <file.txt      # Search in file\n"
               "  echo count <file.txt               # Count lines/words";
    }
    
    CommandResult execute(const std::vector<std::string>& args) override {
        CommandResult result;
        
        if (args.empty()) {
            result.output = "\n"; // 空 echo 输出换行
            return result;
        }
        
        const std::string& first_arg = args[0];
        
        // 子命令：grep, replace, count
        if (first_arg == "grep") {
            return handle_grep(args);
        } else if (first_arg == "replace") {
            return handle_replace(args);
        } else if (first_arg == "count") {
            return handle_count(args);
        } else {
            // 普通 echo：输出文本
            return handle_echo(args);
        }
    }
    
private:
    static CommandResult handle_echo(const std::vector<std::string>& args) {
        CommandResult result;
        
        // 组合所有参数为文本
        std::string text;
        for (size_t i = 0; i < args.size(); i++) {
            if (i > 0) text += " ";
            text += args[i];
        }
        
        // 去除引号
        if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
            text = text.substr(1, text.size() - 2);
        }
        
        result.output = text + "\n";
        return result;
    }
    
    static CommandResult handle_grep(const std::vector<std::string>& args) {
        CommandResult result;
        
        if (args.size() < 2) {
            result.exit_code = 1;
            result.error = "Error: Usage: echo grep \"keyword\" <file>";
            return result;
        }
        
        const std::string& keyword = args[1];
        std::string filepath;
        
        // 查找文件参数（支持 <file 格式）
        for (size_t i = 2; i < args.size(); i++) {
            const std::string& arg = args[i];
            if (arg[0] == '<') {
                filepath = arg.substr(1);
                break;
            }
        }
        
        if (filepath.empty()) {
            result.exit_code = 1;
            result.error = "Error: No file specified.";
            return result;
        }
        
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                result.exit_code = 2;
                result.error = "Error: Cannot open file '" + filepath + "'.";
                return result;
            }
            
            std::string line;
            int line_num = 0;
            while (std::getline(file, line)) {
                line_num++;
                if (line.find(keyword) != std::string::npos) {
                    result.output += std::to_string(line_num) + ": " + line + "\n";
                }
            }
            
        } catch (const std::exception& e) {
            result.exit_code = 3;
            result.error = "Error: " + std::string(e.what());
        }
        
        return result;
    }
    
    static CommandResult handle_replace(const std::vector<std::string>& args) {
        CommandResult result;
        
        if (args.size() < 4) {
            result.exit_code = 1;
            result.error = R"(Error: Usage: echo replace "old" "new" <file> [preview])";
            return result;
        }
        
        const std::string& old_text = args[1];
        const std::string& new_text = args[2];
        std::string filepath;
        bool preview = false;
        
        for (size_t i = 3; i < args.size(); i++) {
            const std::string& arg = args[i];
            if (arg[0] == '<') {
                filepath = arg.substr(1);
            } else if (arg == "preview" || arg == "[preview]") {
                preview = true;
            }
        }
        
        if (filepath.empty()) {
            result.exit_code = 1;
            result.error = "Error: No file specified.";
            return result;
        }
        
        try {
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
            
            // 替换文本
            size_t pos = 0;
            int replace_count = 0;
            while ((pos = content.find(old_text, pos)) != std::string::npos) {
                content.replace(pos, old_text.length(), new_text);
                pos += new_text.length();
                replace_count++;
            }
            
            if (preview) {
                result.output = "Preview (" + std::to_string(replace_count) + " replacements):\n";
                result.output += content;
            } else {
                std::ofstream out(filepath);
                out << content;
                out.close();
                result.output = "Replaced " + std::to_string(replace_count) + " occurrence(s) in " + filepath + "\n";
            }
            
        } catch (const std::exception& e) {
            result.exit_code = 3;
            result.error = "Error: " + std::string(e.what());
        }
        
        return result;
    }
    
    static CommandResult handle_count(const std::vector<std::string>& args) {
        CommandResult result;
        
        if (args.size() < 2) {
            result.exit_code = 1;
            result.error = "Error: Usage: echo count <file>";
            return result;
        }
        
        std::string filepath;
        for (size_t i = 1; i < args.size(); i++) {
            if (args[i][0] == '<') {
                filepath = args[i].substr(1);
                break;
            }
        }
        
        if (filepath.empty()) {
            result.exit_code = 1;
            result.error = "Error: No file specified.";
            return result;
        }
        
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                result.exit_code = 2;
                result.error = "Error: Cannot open file '" + filepath + "'.";
                return result;
            }
            
            int lines = 0, words = 0, chars = 0;
            std::string line;
            while (std::getline(file, line)) {
                lines++;
                chars += line.size() + 1; // +1 for newline
                
                std::istringstream iss(line);
                std::string word;
                while (iss >> word) {
                    words++;
                }
            }
            
            char buf[128];
            snprintf(buf, sizeof(buf), "%d lines, %d words, %d chars\n", lines, words, chars);
            result.output = std::string(buf);
            
        } catch (const std::exception& e) {
            result.exit_code = 3;
            result.error = "Error: " + std::string(e.what());
        }
        
        return result;
    }
};

static struct TextRegistrar {
    TextRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<TextCommand>());
    }
} text_registrar;

} // namespace humanix