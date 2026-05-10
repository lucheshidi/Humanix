#ifndef HUMANIX_DISPATCHER_H
#define HUMANIX_DISPATCHER_H

#include "command.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace humanix {

/**
 * 命令分发器
 * 负责注册、查找和执行命令
 */
class Dispatcher {
public:
    /**
     * 获取单例实例
     */
    static Dispatcher& instance();
    
    /**
     * 注册命令
     * @param cmd 命令对象（智能指针）
     */
    void register_command(std::unique_ptr<Command> cmd);
    
    /**
     * 执行命令
     * @param command_name 命令名称
     * @param args 命令参数
     * @return 执行结果
     */
    CommandResult dispatch(const std::string& command_name, 
                          const std::vector<std::string>& args);
    
    /**
     * 列出所有已注册的命令
     * @return 命令名称列表
     */
    [[nodiscard]] std::vector<std::string> list_commands() const;
    
    /**
     * 获取命令帮助信息
     * @param command_name 命令名称（为空则显示所有命令概览）
     * @return 帮助文本
     */
    [[nodiscard]] std::string get_help(const std::string& command_name = "") const;
    
    /**
     * 检查命令是否存在
     */
    [[nodiscard]] bool has_command(const std::string& command_name) const;

private:
    Dispatcher() = default;
    ~Dispatcher() = default;
    
    // 禁止拷贝和赋值
    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;
    
    std::map<std::string, std::unique_ptr<Command>> commands_;
};

} // namespace humanix

#endif // HUMANIX_DISPATCHER_H
