#include "humanix/command.h"
#include "humanix/dispatcher.h"
#include "humanix/utils.h"

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#else
#include <sys/types.h>
#endif

namespace humanix {

class ProcessCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "process"; }
    
    [[nodiscard]] std::string description() const override {
        return "Process management (list, kill, find, stop)";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "process list [sort=cpu|mem]\n"
               "process kill <pid|name> [force]\n"
               "process find <name>\n"
               "process stop <name>\n"
               "\nExamples:\n"
               "  process list                  # List all processes\n"
               "  process find chrome           # Find chrome processes\n"
               "  process kill 1234             # Kill by PID\n"
               "  process kill chrome [force]   # Kill by name";
    }
    
    CommandResult execute(const std::vector<std::string>& args) override {
        CommandResult result;
        
        if (args.empty()) {
            result.exit_code = 1;
            result.error = "Error: Missing subcommand.\n";
            result.error += "Usage: " + usage();
            return result;
        }
        
        const std::string& subcmd = args[0];
        
        if (subcmd == "list") {
            result = handle_list(args);
        } else if (subcmd == "kill") {
            result = handle_kill(args);
        } else if (subcmd == "find") {
            result = handle_find(args);
        } else if (subcmd == "stop") {
            result = handle_stop(args);
        } else {
            result.exit_code = 1;
            result.error = "Error: Unknown subcommand '" + subcmd + "'.\n";
            result.error += "Usage: " + usage();
        }
        
        return result;
    }
    
private:
    struct ProcessInfo {
        unsigned long pid{};
        std::string name;
        unsigned long memory_kb{};
    };
    
    static CommandResult handle_list(const std::vector<std::string>& args) {
        CommandResult result;
        
#ifdef _WIN32
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            result.exit_code = 1;
            result.error = "Error: Failed to create process snapshot.";
            return result;
        }
        
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        
        if (!Process32First(hSnapshot, &pe)) {
            CloseHandle(hSnapshot);
            result.exit_code = 1;
            result.error = "Error: Failed to enumerate processes.";
            return result;
        }
        
        std::vector<ProcessInfo> processes;
        do {
            ProcessInfo info;
            info.pid = pe.th32ProcessID;
            
            // 直接使用 CHAR 数组
            info.name = std::string(pe.szExeFile);
            
            info.memory_kb = 0; // 简化，不获取内存
            
            processes.push_back(info);
        } while (Process32Next(hSnapshot, &pe));
        
        CloseHandle(hSnapshot);
        
        // 输出
        char header[128];
        snprintf(header, sizeof(header), "%-10s %-30s %s\n", "PID", "NAME", "MEMORY(KB)");
        result.output += std::string(header);
        result.output += std::string(50, '-') + "\n";
        
        for (const auto& proc : processes) {
            char buf[128];
            snprintf(buf, sizeof(buf), "%-10lu %-30s %lu\n", proc.pid, proc.name.c_str(), proc.memory_kb);
            result.output += std::string(buf);
        }
        
        result.output += "\nTotal: " + std::to_string(processes.size()) + " processes\n";
#else
        result.output = "Process list not implemented for Linux yet.\n";
#endif
        
        return result;
    }
    
    CommandResult handle_kill(const std::vector<std::string>& args) {
        CommandResult result;
        
        if (args.size() < 2) {
            result.exit_code = 1;
            result.error = "Error: Missing PID or process name.";
            return result;
        }
        
        const std::string& target = args[1];
        const bool force = utils::has_option(args, "force");
        
#ifdef _WIN32
        // 尝试解析为 PID
        unsigned long pid = 0;
        try {
            pid = std::stoul(target);
        } catch (...) {
            // 不是数字，当作进程名
            pid = find_process_by_name(target);
            if (pid == 0) {
                result.exit_code = 1;
                result.error = "Error: Process '" + target + "' not found.";
                return result;
            }
        }
        
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (!hProcess) {
            result.exit_code = 2;
            result.error = "Error: Cannot open process " + target + ".";
            return result;
        }
        
        if (!TerminateProcess(hProcess, force ? 1 : 0)) {
            CloseHandle(hProcess);
            result.exit_code = 3;
            result.error = "Error: Failed to terminate process.";
            return result;
        }
        
        CloseHandle(hProcess);
        result.output = "Process " + target + " terminated.\n";
#else
        result.output = "Process kill not implemented for Linux yet.\n";
#endif
        
        return result;
    }
    
    static CommandResult handle_find(const std::vector<std::string>& args) {
        CommandResult result;
        
        if (args.size() < 2) {
            result.exit_code = 1;
            result.error = "Error: Missing process name.";
            return result;
        }
        
        const std::string& name = args[1];
        
#ifdef _WIN32
        const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            result.exit_code = 1;
            result.error = "Error: Failed to create process snapshot.";
            return result;
        }
        
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        
        int count = 0;
        if (Process32First(hSnapshot, &pe)) {
            do {
                std::string proc_name(pe.szExeFile);
                if (proc_name.find(name) != std::string::npos) {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "%-10lu %s\n", pe.th32ProcessID, pe.szExeFile);
                    result.output += std::string(buf);
                    count++;
                }
            } while (Process32Next(hSnapshot, &pe));
        }
        
        CloseHandle(hSnapshot);
        
        if (count == 0) {
            result.output = "No processes found matching '" + name + "'.\n";
        } else {
            result.output += "\nFound " + std::to_string(count) + " process(es).\n";
        }
#else
        result.output = "Process find not implemented for Linux yet.\n";
#endif
        
        return result;
    }
    
    static CommandResult handle_stop(const std::vector<std::string>& args) {
        CommandResult result;
        result.error = "Warning: 'stop' subcommand not implemented. Use 'kill' instead.";
        result.exit_code = 1;
        return result;
    }
    
#ifdef _WIN32
    static unsigned long find_process_by_name(const std::string& name) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) return 0;
        
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        
        unsigned long pid = 0;
        if (Process32First(hSnapshot, &pe)) {
            do {
                std::string proc_name(pe.szExeFile);
                if (proc_name.find(name) != std::string::npos) {
                    pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe));
        }
        
        CloseHandle(hSnapshot);
        return pid;
    }
#endif
};

static struct ProcessRegistrar {
    ProcessRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<ProcessCommand>());
    }
} process_registrar;

} // namespace humanix