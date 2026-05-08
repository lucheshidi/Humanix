#include "humanix/command.h"
#include "humanix/dispatcher.h"
#include <thread>
#include <future>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>

namespace humanix {

// 简单的任务管理器
class JobManager {
public:
    static JobManager& instance() {
        static JobManager mgr;
        return mgr;
    }
    
    int add_job(const std::string& command, std::future<void> future) {
        std::lock_guard<std::mutex> lock(mutex_);
        int job_id = next_job_id_++;
        jobs_[job_id] = {command, std::move(future), JobStatus::RUNNING};
        return job_id;
    }
    
    void list_jobs() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (jobs_.empty()) {
            std::cout << "No jobs.\n";
            return;
        }
        
        for (const auto& [id, job] : jobs_) {
            std::string status_str;
            switch (job.status) {
                case JobStatus::RUNNING: status_str = "Running"; break;
                case JobStatus::COMPLETED: status_str = "Done"; break;
                case JobStatus::FAILED: status_str = "Failed"; break;
            }
            std::cout << "[" << id << "] " << status_str << "  " << job.command << "\n";
        }
    }
    
    void cleanup_finished() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = jobs_.begin(); it != jobs_.end();) {
            if (it->second.status != JobStatus::RUNNING) {
                it = jobs_.erase(it);
            } else {
                ++it;
            }
        }
    }
    
private:
    enum class JobStatus { RUNNING, COMPLETED, FAILED };
    
    struct Job {
        std::string command;
        std::future<void> future;
        JobStatus status;
    };
    
    std::mutex mutex_;
    std::map<int, Job> jobs_;
    int next_job_id_ = 1;
};

class JobsCommand : public Command {
public:
    [[nodiscard]] std::string name() const override { return "jobs"; }
    
    [[nodiscard]] std::string description() const override {
        return "List background jobs";
    }
    
    [[nodiscard]] std::string usage() const override {
        return "jobs\n"
               "\nExamples:\n"
               "  jobs          # List all background jobs";
    }
    
    CommandResult execute(const std::vector<std::string>& args) override {
        CommandResult result;
        JobManager::instance().cleanup_finished();
        
        // 直接输出到 result.output
        std::ostringstream oss;
        auto& mgr = JobManager::instance();
        
        // 这里简化处理，实际应该捕获输出
        result.output = "(Jobs listing not fully implemented in this version)\n";
        
        return result;
    }
};

static struct JobsRegistrar {
    JobsRegistrar() {
        Dispatcher::instance().register_command(std::make_unique<JobsCommand>());
    }
} jobs_registrar;

} // namespace humanix
