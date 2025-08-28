#pragma once

#include <array>

#include <common/types.h>

#include "cpu.h"
#include "mmu.h"

class Bus {
public:
    Bus();

    u8 memRead(u16 address) const;
    void memWrite(u16 address, u8 value);
    u8& getMemRef(u16 address);
    const std::array<u8, 1024 * 7>& getFrameBuffer() const;

    u8 in(u8 portNum) const;
    void out(u8 portNum, u8 value);
private:
    CPU cpu;
    MMU mmu;

    u8 port1 {}, port2 {}, port3 {}, port5 {};

    u8 shiftAmount {};
    u8 shiftOffset {};
    u16 shiftRegister {};
};