cmake_minimum_required(VERSION 3.20)
project(humanix
    VERSION 1.0.0
    DESCRIPTION "A modern, human-friendly command-line toolset to replace POSIX utilities"
    LANGUAGES CXX
)

# C++20 标准设置
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项（跨平台兼容）
if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 包含目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 收集所有源文件（自动扫描，避免手动维护）
file(GLOB_RECURSE HUMANIX_SOURCES
    "src/humanix/*.cpp"
    "src/commands/*.cpp"
    "src/lib/*.cpp"
    "src/shell/*.cpp"
)

# 主可执行文件
add_executable(humanix ${HUMANIX_SOURCES})

# 输出目录配置（可选，便于调试）
set_target_properties(humanix PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/release
)

# 安装规则（可选）
install(TARGETS humanix DESTINATION bin)
