#pragma once

#include <array>
#include <span>

#include "types.h"

// Cyclic Redundancy Check (CRC) algorithms for checking file integrity
// Based on the implementation described at https://wiki.osdev.org/CRC32

namespace crc {
    namespace internal {
        constexpr std::array<u32, 256> generateCrc32Table() {
            std::array<u32, 256> table {};
            for (u32 i{0}; i < 256; i++) {
                u32 crc {i};
                for (int j{0}; j < 8; j++) {
                    crc = (crc & 1) ? (0xEDB88320 ^ (crc >> 1)) : (crc >> 1);
                }
                table[i] = crc;
            }
            return table;
        }

        constexpr auto crc32Table = generateCrc32Table();
    }

    constexpr u32 crc32(std::span<const u8> data) {
        u32 checksum {0xFFFFFFFF};

        for (const u8 byte : data) {
            checksum = internal::crc32Table[static_cast<u8>(checksum) ^ byte] ^ (checksum >> 8);
        }

        return checksum ^ 0xFFFFFFFF;
    }
}