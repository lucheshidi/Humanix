# Humanix Standard v1.0

## 1. 宣言

POSIX 命令系统过于反人类：缩写混乱、选项不一致、默认行为危险。我们设计 Humanix，目标是打造一套**更简洁、更一致、更安全、更现代**的命令行标准，并最终实现一个可完全替换 Busybox 的轻量核心工具集。

## 2. 核心设计原则

- 命令名优先短而清晰
- 选项尽量放在命令最前面（[option] 形式）
- 支持智能默认 + 显式选项
- 保留常用短别名，降低迁移成本
- 危险操作默认安全，需要显式开启

## 3. 核心命令列表

### 创建
```humanix
crt [d] [f] [force] <name>

- 无选项：智能模式（带常见后缀 → 文件，无后缀 → 目录）
- [d]：强制创建目录（默认递归创建中间目录）
- [f]：强制创建文件
- [d] 和 [f] 同时出现：报错
```

### 文件/目录操作
```humanix
list [human] [sort=name|size|time] [long] <path>  
copy [r] [force] <src> <dst>  
move [force] <src> <dst>  
delete [r] [force] [preview] <target>...
```

### 查看
```humanix
show [page] [follow] <file>
```

### 导航
```humanix
cd <path>                  # 支持 .. / ... / .... 等向上多级  
cd -                       # 返回上一个目录
```

### 进程（统一）
```humanix
process list [sort=cpu|mem]  
process kill <pid|name> [force]  
process find <name>  
process stop <name>
```

### 权限（简化）
```humanix
perm [r] <mode> <target>  
own  [r] <owner> <target>
```

### 文本处理（统一为 echo）
```humanix
echo "内容"  
echo "内容" > file.txt  
echo "内容" >> file.txt  
echo grep "keyword" <file>  
echo replace "old" "new" <file> [preview]  
echo count <file>
```

### 搜索
```humanix
find [name=*.log] [content=关键词] [path=.]
```

### 系统信息
```humanix
sys info [cpu|mem|disk]  
sys uptime
```

### 其他
```humanix
help [command]  
exit
```

## 4. 参数风格推荐

**首选风格（选项靠前）**：
```humanix
crt [d] myproject
delete [r] [force] tempdir
list [human] [sort=size]
copy [r] src dst
```

**备选风格**：
delete tempdir recursive=true
list /home human=true

## 5. 未来扩展方向

- 完整 POSIX 兼容层（legacy 模式）
- 中文别名支持（可选）
- 结构化输出（--json）
- 回收站机制（delete 默认进入回收站）
