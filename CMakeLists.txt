# Root CMake configuration for tau-programming-lang
cmake_minimum_required(VERSION 3.16)

project(tau-programming-lang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect all sources (simplify if you prefer explicit lists)
file(GLOB_RECURSE SRC_FILES
    "tau-programming-lang/overloads/*.cpp"
    "tau-programming-lang/Statements/*.cpp"
    "tau-programming-lang/Expression/*.cpp"
    "tau-programming-lang/Visitor/*.cpp"
    "tau-programming-lang/ExecutionContext/*.cpp"
    "tau-programming-lang/builtin/*.cpp"
    "tau-programming-lang/Interpreter/*.cpp"
    "tau-programming-lang/ExpressionVisitor/*.cpp"
    "tau-programming-lang/Parser/*.cpp"
    "tau-programming-lang/Scanner/*.cpp"
    "tau-programming-lang/Visitor/*.cpp"
    "tau-programming-lang/*.cpp"
)

add_executable(tau_interpreter ${SRC_FILES})

# Include directories
target_include_directories(tau_interpreter PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
)

# OS-specific libraries
if(APPLE)
    message(STATUS "Building on macOS")
    # target_link_libraries(tau_interpreter PRIVATE "-framework Cocoa")
elseif(WIN32)
    message(STATUS "Building on Windows")
    target_link_libraries(tau_interpreter PRIVATE ws2_32)
endif()
