#pragma once

#include <array>

#include <common/types.h>

class MMU {
public:
    void loadRom();
    u8 read(u16 address) const;
    void write(u16 address, u8 value);
    u8& getRef(u16 address);
    const std::array<u8, 1024 * 7>& getFrameBuffer() const;
private:
    std::array<u8, 1024 * 8> rom {};
    std::array<u8, 1024> ram {};
    std::array<u8, 1024 * 7> vram {};
};