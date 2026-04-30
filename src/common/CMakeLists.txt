add_library(common STATIC
    bits.h
    crc.h
    log.h
    types.h
)

target_link_libraries(common
    INTERFACE
        fmt::fmt
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)