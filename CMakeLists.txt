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
add_executable(humanix ${HUMANIX_SOURCES}
        src/commands/jobs.cpp
        src/commands/env.cpp)

# 静态链接选项（可选）
option(BUILD_STATIC "Build static binary" OFF)
if(BUILD_STATIC)
    if(UNIX)
        target_link_options(humanix PRIVATE -static)
        message(STATUS "Building static binary")
    else()
        message(WARNING "Static build only supported on Unix systems")
    endif()
endif()

# Linux 下使用 FetchContent 获取 readline（如果系统没有）
if(UNIX AND NOT APPLE)
    find_library(READLINE_LIB NAMES readline libreadline)
    find_library(HISTORY_LIB NAMES history libhistory)
    
    if(NOT READLINE_LIB OR NOT HISTORY_LIB)
        # 系统没有 readline，使用 FetchContent
        message(STATUS "readline not found, fetching from source...")
        include(FetchContent)
        
        # 注意：readline 依赖 ncurses，这里简化处理
        # 实际项目中可能需要更复杂的依赖处理
        find_package(Curses QUIET)
        if(NOT CURSES_FOUND)
            message(WARNING "ncurses not found. readline support disabled.")
        else()
            # readline 编译复杂，建议用户安装系统库
            message(WARNING "Please install libreadline-dev:")
            message(WARNING "  Ubuntu/Debian: sudo apt install libreadline-dev")
            message(WARNING "  RedHat/Fedora: sudo dnf install readline-devel")
        endif()
    else()
        # 使用系统库
        target_link_libraries(humanix PRIVATE ${READLINE_LIB} ${HISTORY_LIB})
        target_compile_definitions(humanix PRIVATE HAVE_READLINE)
        message(STATUS "Found readline: ${READLINE_LIB}")
    endif()
endif()

# 输出目录配置（可选，便于调试）
set_target_properties(humanix PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/release
)

# 安装规则（可选）
install(TARGETS humanix DESTINATION bin)
