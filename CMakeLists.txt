cmake_minimum_required(VERSION 3.28)

project(
  pocket-interpreter
  VERSION 0.1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Useful SDL extensions.
set(SDLIMAGE_JPG ON)
set(SDLIMAGE_PNG ON)

# Disable dependency on XScreenSaver for compilation on WSL.
set(SDL_X11_XSCRNSAVER OFF)

include(FetchContent)

# Pull in Corrosion build tool for compiling with Rust.
FetchContent_Declare(
  Corrosion
  GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
  GIT_TAG v0.6 # Optionally specify a commit hash, version tag or branch here
)
FetchContent_MakeAvailable(Corrosion)

# Add external dependencies via Git submodules.
add_subdirectory(lib/SDL EXCLUDE_FROM_ALL)
add_subdirectory(lib/SDL_image EXCLUDE_FROM_ALL)

# Import targets defined in the Pocket Compiler library.
corrosion_import_crate(MANIFEST_PATH lib/pocket-compiler/Cargo.toml
                       CARGO_TARGET_TRIPLE ${CMAKE_C_COMPILER_TARGET})

add_library(pocket-interpreter src/interpreter.cpp)

# Include Pocket Interpreter headers.
target_include_directories(pocket-interpreter
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Compile with C++ version set.
target_compile_features(pocket-interpreter PUBLIC cxx_std_20)

# Link to the actual SDL3 libraries.
target_link_libraries(pocket-interpreter PRIVATE SDL3::SDL3
                                                 SDL3_image::SDL3_image)

# Compile Pocket Interpreter executable frontend.
add_executable(pocket-interpreter_cli test/main.cpp)

# Include asset directory.
target_compile_definitions(pocket-interpreter_cli
                           PRIVATE ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")

# Link against Pocket Interpreter core.
target_link_libraries(pocket-interpreter_cli PRIVATE pocket-interpreter)

# Link against the Pocket Compiler.
target_link_libraries(pocket-interpreter_cli PUBLIC compiler_ffi)
