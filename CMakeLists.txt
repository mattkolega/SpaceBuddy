cmake_minimum_required(VERSION 3.31)

project(
    SpaceBuddy
    VERSION 0.1.0
    DESCRIPTION "Space Invaders arcade emulator"
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/ProjectConfig.cmake)

include(FetchContent)
# Get fmt library for formatting console output
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 12.1.0
    GIT_SHALLOW TRUE
)
# Get SDL3 for multimedia functionality
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.10
    GIT_SHALLOW TRUE
)
# Get miniaudio for audio playback
FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git
    GIT_TAG 0.11.25
    GIT_SHALLOW TRUE
)

# Stop SDL from compiling tests
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(fmt SDL3 miniaudio)

add_subdirectory(src/common)
add_subdirectory(src/core)

add_executable(spacebuddy
    src/main.cpp
    src/app.cpp
    src/audio.cpp
    src/platform.cpp
    src/renderer.cpp
)

target_link_libraries(spacebuddy
    PRIVATE
        project_sanitizers
        project_warnings
        common
        core
        SDL3::SDL3
        miniaudio
)

# Copy assets to build dir
add_custom_command(
    TARGET spacebuddy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:spacebuddy>/assets
)
