#pragma once

#include <array>
#include <span>
#include <vector>

#include "common/types.h"

// MMU Memory Map
// ---
// 0000-1FFF 8K ROM
// 2000-23FF 1K RAM
// 2400-3FFF 7K VRAM
// 4000-43FF RAM mirror
// 4400-5FFF VRAM mirror

// Handles memory read and write operations
class MMU {
public:
    static constexpr int ROM_SIZE  {1024 * 8};
    static constexpr int RAM_SIZE  {1024};
    static constexpr int VRAM_SIZE {1024 * 7};

    bool loadBufferIntoRom(const std::vector<u8>& buffer);
    u8 read(u16 address) const;
    void write(u16 address, u8 value);
    std::span<const u8, VRAM_SIZE> getVram() const;
private:
    std::array<u8, ROM_SIZE>  m_rom  {};
    std::array<u8, RAM_SIZE>  m_ram  {};
    std::array<u8, VRAM_SIZE> m_vram {};
};
