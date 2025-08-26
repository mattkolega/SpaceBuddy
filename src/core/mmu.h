#pragma once

#include <array>

#include <common/types.h>

class MMU {
private:
    std::array<u8, 1024 * 64> memory {};
public:
    u8 memRead(u16 address) const { return memory[address]; }
    u8& get(u16 address) { return memory[address]; }
    void memWrite(u16 address, u8 value) { memory[address] = value; }
};