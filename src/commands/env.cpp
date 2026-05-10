#include "humanix/command.h"
#include "humanix/dispatcher.h"
#include <cstdlib>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace humanix {

class EnvCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "env"; }
    
    [[nodiscard]] std::string description() const override {
        return "Display or set environment variables";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "env [name=value] [name]\n"
               "\nExamples:\n"
               "  env                    # List all variables\n"
               "  env PATH               # Show specific variable\n"
               "  env MYVAR=hello        # Set variable (session only)";
    }
    
    CommandResult execute(const std::vector<std::string>& args) override {
        CommandResult result;

        if (args.empty()) {
            // 列出所有环境变量
#ifdef _WIN32
            // Windows: 使用 GetEnvironmentStringsA (ANSI版本)
            if (LPCH env_strings = GetEnvironmentStringsA()) {
                LPCH ptr = env_strings;
                while (*ptr) {
                    result.output += std::string(ptr) + "\n";
                    ptr += strlen(ptr) + 1;
                }
                FreeEnvironmentStringsA(env_strings);
            }
#else
            // Linux/Unix: environ 全局变量
            extern char** environ;
            for (char** env = ::environ; *env; ++env) {
                result.output += std::string(*env) + "\n";
            }
#endif
        } else {
            for (const auto& arg : args) {
                // 检查是否是赋值操作
                auto eq_pos = arg.find('=');
                if (eq_pos != std::string::npos) {
                    // 设置环境变量
                    std::string name = arg.substr(0, eq_pos);
                    std::string value = arg.substr(eq_pos + 1);
                    
#ifdef _WIN32
                    SetEnvironmentVariableA(name.c_str(), value.c_str());
#else
                    setenv(name.c_str(), value.c_str(), 1);
#endif
                    result.output += "Set: " + name + "=" + value + "\n";
                } else {
                    // 查询环境变量
                    const char* value = std::getenv(arg.c_str());
                    if (value) {
                        result.output += std::string(value) + "\n";
                    } else {
                        result.error += "env: " + arg + ": Undefined variable\n";
                    }
                }
            }
        }
        
        return result;
    }
};

static struct EnvRegistrar {
    EnvRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<EnvCommand>());
    }
} env_registrar;

} // namespace humanix
