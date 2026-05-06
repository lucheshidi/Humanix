#ifndef HUMANIX_COMMAND_H
#define HUMANIX_COMMAND_H

#include <string>
#include <vector>
#include <optional>

namespace humanix {

/**
 * 命令执行结果
 */
struct CommandResult {
    int exit_code = 0;              // 退出码：0=成功，非0=失败
    std::string output;             // 标准输出
    std::string error;              // 错误信息
    bool success() const { return exit_code == 0; }
};

/**
 * 命令基类
 * 所有命令都需要继承此类并实现 execute 方法
 */
class Command {
public:
    virtual ~Command() = default;
    
    /**
     * 执行命令
     * @param args 命令行参数（不包括命令名本身）
     * @return 执行结果
     */
    virtual CommandResult execute(const std::vector<std::string>& args) = 0;
    
    /**
     * 获取命令名称
     */
    virtual std::string name() const = 0;
    
    /**
     * 获取命令简短描述
     */
    virtual std::string description() const = 0;
    
    /**
     * 获取命令用法说明
     */
    virtual std::string usage() const = 0;
};

} // namespace humanix

#endif // HUMANIX_COMMAND_H
