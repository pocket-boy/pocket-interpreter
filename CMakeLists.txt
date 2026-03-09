cmake_minimum_required(VERSION 3.28)
project(pocket-interpreter VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(pocket-interpreter src/interpreter.cpp)

target_include_directories(pocket-interpreter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(pocket-interpreter PUBLIC cxx_std_20)

#Find locally for testing without editor
if(NOT TARGET SDL3::SDL3)
    find_package(SDL3 CONFIG REQUIRED)
endif()
if(NOT TARGET SDL3_image::SDL3_image)
    find_package(SDL3_image CONFIG REQUIRED)
endif()

target_link_libraries(pocket-interpreter
    PRIVATE SDL3::SDL3 
            SDL3_image::SDL3_image
)

# For testing reasons.
if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    add_executable(
        pocket-interpreter_cli
        test/main.cpp
    )

    target_compile_definitions(pocket-interpreter_cli
        PRIVATE ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
    )

    target_link_libraries(pocket-interpreter_cli
        PRIVATE pocket-interpreter
    )
endif()