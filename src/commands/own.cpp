#include "humanix/command.h"
#include "humanix/dispatcher.h"

namespace humanix {

class OwnCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "own"; }
    
    [[nodiscard]] std::string description() const override {
        return "Change file/directory ownership (Linux only)";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "own [r] <owner> <target>\n"
               "\nOptions:\n"
               "  [r]  Recursive (for directories)\n"
               "\nExamples:\n"
               "  own user file.txt            # Change owner\n"
               "  own [r] user docs/           # Recursive";
    }
    
    CommandResult execute(const std::vector<std::string>& args) override {
        CommandResult result;
        
#ifdef _WIN32
        result.error = "Warning: Ownership management is not applicable on Windows.\n";
        result.error += "This command is designed for Unix/Linux systems.";
        result.exit_code = 1;
#else
        // Linux 实现待完成
        result.error = "Warning: Ownership command not fully implemented yet.";
        result.exit_code = 1;
#endif
        
        return result;
    }
};

static struct OwnRegistrar {
    OwnRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<OwnCommand>());
    }
} own_registrar;

} // namespace humanix