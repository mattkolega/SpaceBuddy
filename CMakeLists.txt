cmake_minimum_required(VERSION 3.31.6)

project(
    SpaceBuddy
    VERSION 0.1.0
    DESCRIPTION "Space Invaders arcade emulator"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For intellisense

include(FetchContent)
# Get fmt library for formatting console output
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 12.1.0
)
# Get SDL3 for multimedia functionality
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.4
)

# Stop SDL from compiling tests
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(fmt SDL3)

add_subdirectory(src/common)
add_subdirectory(src/core)

add_executable(spacebuddy
    src/main.cpp
    src/app.cpp
    src/platform.cpp
    src/renderer.cpp
)

target_link_libraries(spacebuddy
    PRIVATE
        common
        core
        SDL3::SDL3
)