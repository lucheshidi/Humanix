#include "humanix/command.h"
#include "humanix/dispatcher.h"
#include "humanix/utils.h"

namespace humanix {

class PermCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "perm"; }
    
    [[nodiscard]] std::string description() const override {
        return "Change file/directory permissions (Linux only)";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "perm [r] <mode> <target>\n"
               "\nOptions:\n"
               "  [r]  Recursive (for directories)\n"
               "\nExamples:\n"
               "  perm 755 script.sh           # Set permissions\n"
               "  perm [r] 644 docs/           # Recursive";
    }
    
    CommandResult execute(const std::vector<std::string>& args) override {
        CommandResult result;
        
#ifdef _WIN32
        result.error = "Warning: Permission management is not applicable on Windows.\n";
        result.error += "This command is designed for Unix/Linux systems.";
        result.exit_code = 1;
#else
        // Linux 实现待完成
        result.error = "Warning: Permission command not fully implemented yet.";
        result.exit_code = 1;
#endif
        
        return result;
    }
};

static struct PermRegistrar {
    PermRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<PermCommand>());
    }
} perm_registrar;

} // namespace humanix