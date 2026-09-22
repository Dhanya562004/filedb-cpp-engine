cmake_minimum_required(VERSION 3.12)
project(filedb_cpp_engine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES 
    src/Models.cpp
    src/StorageManager.cpp
    src/IndexManager.cpp
    src/Logger.cpp
    src/QueryAnalyzer.cpp
    src/Parser.cpp
    src/ExecutionEngine.cpp
    src/NlpToSql.cpp
)

# Main database CLI executable
add_executable(filedb ${SOURCES} src/main.cpp)

# Automated unit test runner
add_executable(test_runner ${SOURCES} src/test_runner.cpp)
