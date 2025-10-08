# Root CMake configuration for ardan-lang
cmake_minimum_required(VERSION 3.16)

project(ardan-lang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all sources (simplify if you prefer explicit lists)
file(GLOB_RECURSE SRC_FILES
    "ardan-lang/overloads/*.cpp"
    "ardan-lang/Statements/*.cpp"
    "ardan-lang/Expression/*.cpp"
    "ardan-lang/Visitor/*.cpp"
    "ardan-lang/ExecutionContext/*.cpp"
    "ardan-lang/builtin/*.cpp"
    "ardan-lang/Interpreter/*.cpp"
    "ardan-lang/ExpressionVisitor/*.cpp"
    "ardan-lang/Parser/*.cpp"
    "ardan-lang/Scanner/*.cpp"
    "ardan-lang/Visitor/*.cpp"
    "ardan-lang/REPL/*.cpp"
    "ardan-lang/JSON/*.cpp"
    "ardan-lang/EventLoop/*.cpp"
    "ardan-lang/Compiler/*.cpp"
    "ardan-lang/*.cpp"
)

add_executable(ardan ${SRC_FILES})

# Include directories
target_include_directories(ardan PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
)

# OS-specific libraries
if(APPLE)
    message(STATUS "Building on macOS")
    target_link_libraries(ardan PRIVATE "-framework Cocoa")
elseif(WIN32)
    message(STATUS "Building on Windows")
    target_link_libraries(ardan PRIVATE ws2_32)
endif()
