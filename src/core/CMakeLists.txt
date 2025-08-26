add_library(core STATIC
    cpu.cpp
    mmu.h
)

target_link_libraries(core
    PRIVATE
        common
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)