add_library(core STATIC
    cpu.cpp
    mmu.cpp
    spaceinvaders.cpp
)

target_link_libraries(core
    PRIVATE
        project_sanitizers
        project_warnings
        common
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
