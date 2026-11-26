cmake_minimum_required(VERSION 3.28)

project(Pluribus VERSION 0.1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(PluribusCompilerOptions)

option(PLURIBUS_BUILD_TESTS "Build Pluribus tests" ON)
option(PLURIBUS_BUILD_CODEC_BENCH "Build the control-codec benchmark (downloads dependencies)" OFF)
option(PLURIBUS_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(core)
add_subdirectory(fabric)
add_subdirectory(master/scheduler)

if(PLURIBUS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(PLURIBUS_BUILD_CODEC_BENCH)
    add_subdirectory(benchmarks/codec)
endif()
