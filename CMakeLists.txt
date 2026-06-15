cmake_minimum_required(VERSION 3.16)
project(ccj VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

option(CCJ_SIMULATE "Build with simulated sensor values (no hardware required)" OFF)

file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/**/*.cpp
)

add_executable(ccj ${SOURCES})

target_include_directories(ccj PRIVATE
    src
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(ccj PRIVATE
    ${SDL2_LIBRARIES}
)

if(CCJ_SIMULATE)
    target_compile_definitions(ccj PRIVATE CCJ_SIMULATE)
    message(STATUS "CCJ: simulation mode enabled")
endif()
