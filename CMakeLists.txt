cmake_minimum_required(VERSION 3.5.2)
project(
    updater_core
    LANGUAGES CXX
    VERSION 1.0.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(UPDATER_ENABLE_TESTING "Specify if testing should be enabled" OFF)
option(UPDATER_ENABLE_CODE_COVERAGE
    "Specify if code coverage should be enabled" OFF)
option(UPDATER_EXTERNAL_SPDLOG
    "Specify whether to use an external spdlog library" OFF)
option(UPDATER_EXTERNAL_GTEST
    "Specify whether to use an external googletest library" OFF)
set(LOG_LEVELS
    TRACE
    DEBUG
    INFO
    WARN
    ERROR
    CRITICAL
    OFF)

set(UPDATER_LOG_LEVEL
    "TRACE"
    CACHE STRING "Specify spdlog SPDLOG_ACTIVE_LEVEL ${LOG_LEVELS}")

if(UPDATER_ENABLE_TESTING)
    if(UPDATER_ENABLE_CODE_COVERAGE)
        include(CodeCoverage)
        append_coverage_compiler_flags()
    endif()

    if(NOT UPDATER_EXTERNAL_GTEST)
        add_subdirectory(dep/external/gtest)
    endif()
endif()

if(NOT UPDATER_EXTERNAL_SPDLOG)
    add_subdirectory(dep/external/spdlog)
endif()

add_subdirectory(dep/internal/ipc)

find_package(Filesystem REQUIRED COMPONENTS Final Experimental)

add_library(${PROJECT_NAME} STATIC src/app.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC ipc spdlog::spdlog std::filesystem)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

list(FIND LOG_LEVELS ${UPDATER_LOG_LEVEL} LOG_LEVEL_INDEX)

if(LOG_LEVEL_INDEX EQUAL -1)
    message(
        FATAL_ERROR
        "Invalid LOG_LEVEL: ${UPDATER_LOG_LEVEL}. Allowed values are ${LOG_LEVELS}."
    )
endif()

message("Setting LOG_LEVEL to ${UPDATER_LOG_LEVEL}")
target_compile_definitions(${PROJECT_NAME}
    PRIVATE SPDLOG_ACTIVE_LEVEL=${LOG_LEVEL_INDEX})

if(UPDATER_ENABLE_TESTING)
    add_subdirectory(test/unit)
endif()
