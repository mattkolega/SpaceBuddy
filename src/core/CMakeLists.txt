add_library(core STATIC
    bus.cpp
    cpu.cpp
    mmu.cpp
)

target_link_libraries(core
    PRIVATE
        common
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)