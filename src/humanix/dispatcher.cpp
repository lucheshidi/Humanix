#include "humanix/dispatcher.h"
#include <sstream>
#include <algorithm>

namespace humanix {

Dispatcher& Dispatcher::instance() {
    static Dispatcher instance;
    return instance;
}

void Dispatcher::register_command(std::unique_ptr<Command> cmd) {
    if (cmd) {
        commands_[cmd->name()] = std::move(cmd);
    }
}

CommandResult Dispatcher::dispatch(const std::string& command_name,
                                   const std::vector<std::string>& args) {
    auto it = commands_.find(command_name);
    if (it == commands_.end()) {
        CommandResult result;
        result.exit_code = 1;
        result.error = "Unknown command: " + command_name + "\n";
        result.error += "Type 'help' to see available commands.";
        return result;
    }
    
    try {
        return it->second->execute(args);
    } catch (const std::exception& e) {
        CommandResult result;
        result.exit_code = 2;
        result.error = std::string("Command execution error: ") + e.what();
        return result;
    } catch (...) {
        CommandResult result;
        result.exit_code = 3;
        result.error = "Unknown error occurred during command execution.";
        return result;
    }
}

std::vector<std::string> Dispatcher::list_commands() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : commands_) {
        names.push_back(name);
    }
    return names;
}

std::string Dispatcher::get_help(const std::string& command_name) const {
    std::ostringstream oss;
    
    if (command_name.empty()) {
        // 显示所有命令概览
        oss << "Humanix v1.0 - Modern Command-Line Tools\n\n";
        oss << "Available commands:\n";
        
        for (const auto& [name, cmd] : commands_) {
            oss << "  " << name << "\t- " << cmd->description() << "\n";
        }
        
        oss << "\nType 'help <command>' for more information on a specific command.\n";
    } else {
        // 显示特定命令的详细帮助
        auto it = commands_.find(command_name);
        if (it == commands_.end()) {
            return "No help available for unknown command: " + command_name;
        }
        
        oss << "Command: " << it->second->name() << "\n";
        oss << "Description: " << it->second->description() << "\n";
        oss << "Usage: " << it->second->usage() << "\n";
    }
    
    return oss.str();
}

bool Dispatcher::has_command(const std::string& command_name) const {
    return commands_.find(command_name) != commands_.end();
}

} // namespace humanix