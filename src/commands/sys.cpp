#include "humanix/command.h"
#include "humanix/dispatcher.h"
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#endif

namespace humanix {

class SysCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "sys"; }
    
    [[nodiscard]] std::string description() const override {
        return "Display system information";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "sys info [cpu|mem|disk]\n"
               "sys uptime\n"
               "\nExamples:\n"
               "  sys info          # Show all system info\n"
               "  sys info cpu      # CPU info only\n"
               "  sys uptime        # System uptime";
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
        
        if (subcmd == "uptime") {
            result.output = get_uptime();
        } else if (subcmd == "info") {
            std::string filter = args.size() > 1 ? args[1] : "";
            result.output = get_system_info(filter);
        } else {
            result.exit_code = 1;
            result.error = "Error: Unknown subcommand '" + subcmd + "'.\n";
            result.error += "Usage: " + usage();
        }
        
        return result;
    }
    
private:
    static std::string get_uptime() {
#ifdef _WIN32
        DWORD uptime_ms = GetTickCount();
        auto seconds = uptime_ms / 1000;
        auto hours = seconds / 3600;
        auto minutes = (seconds % 3600) / 60;
        auto secs = seconds % 60;
        
        char buf[128];
        snprintf(buf, sizeof(buf), "System uptime: %lu hours, %lu minutes, %lu seconds\n", 
                 hours, minutes, secs);
        return std::string(buf);
#else
        struct sysinfo info;
        sysinfo(&info);
        
        auto seconds = info.uptime;
        auto hours = seconds / 3600;
        auto minutes = (seconds % 3600) / 60;
        auto secs = seconds % 60;
        
        char buf[128];
        snprintf(buf, sizeof(buf), "System uptime: %lu hours, %lu minutes, %lu seconds\n", 
                 hours, minutes, secs);
        return std::string(buf);
#endif
    }
    
    static std::string get_system_info(const std::string& filter) {
        std::string output;
        
        if (filter.empty() || filter == "cpu") {
            output += "CPU Information:\n";
#ifdef _WIN32
            SYSTEM_INFO sysInfo;
            GetSystemInfo(&sysInfo);
            output += "  Processors: " + std::to_string(sysInfo.dwNumberOfProcessors) + "\n";
            output += "  Architecture: ";
            switch (sysInfo.wProcessorArchitecture) {
                case PROCESSOR_ARCHITECTURE_AMD64: output += "x64\n"; break;
                case PROCESSOR_ARCHITECTURE_INTEL: output += "x86\n"; break;
                case PROCESSOR_ARCHITECTURE_ARM64: output += "ARM64\n"; break;
                default: output += "Unknown\n"; break;
            }
#else
            output += "  (CPU info not implemented for Linux yet)\n";
#endif
            output += "\n";
        }
        
        if (filter.empty() || filter == "mem") {
            output += "Memory Information:\n";
#ifdef _WIN32
            MEMORYSTATUSEX memStatus;
            memStatus.dwLength = sizeof(memStatus);
            GlobalMemoryStatusEx(&memStatus);
            
            char buf[256];
            snprintf(buf, sizeof(buf), 
                "  Total: %.2f GB\n  Used: %.2f GB (%lu%%)\n  Free: %.2f GB\n",
                memStatus.ullTotalPhys / (1024.0 * 1024 * 1024),
                (memStatus.ullTotalPhys - memStatus.ullAvailPhys) / (1024.0 * 1024 * 1024),
                memStatus.dwMemoryLoad,
                memStatus.ullAvailPhys / (1024.0 * 1024 * 1024));
            output += std::string(buf);
#else
            output += "  (Memory info not implemented for Linux yet)\n";
#endif
            output += "\n";
        }
        
        if (filter.empty() || filter == "disk") {
            output += "Disk Information:\n";
            output += "  (Disk info not implemented yet)\n\n";
        }
        
        return output;
    }
};

static struct SysRegistrar {
    SysRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<SysCommand>());
    }
} sys_registrar;

} // namespace humanix