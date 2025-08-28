#include "mmu.h"

#include <common/logger.h>
#include <common/types.h>

u8 MMU::read(u16 address) const {
    if (address >= 0x0 && address <= 0x1FFF) {
        return rom[address];
    } else if (address >= 0x2000 && address <= 0x23FF) {
        return ram[address];
    } else if (address >= 0x2400 && address <= 0x3FFF) {
        return vram[address];
    } else if (address >= 0x4000 && address <= 0x43FF) {
        return ram[address];
    } else {
        Logger::warn("Read from undefined memory address: {:X}", address);
        return 0;
    }
}

void MMU::write(u16 address, u8 value) {
    if (address >= 0x0 && address <= 0x1FFF) {
        Logger::warn("Illegal write to ROM at address: {:X}", address);
    } else if (address >= 0x2000 && address <= 0x23FF) {
        ram[address] = value;
    } else if (address >= 0x2400 && address <= 0x3FFF) {
        vram[address] = value;
    } else if (address >= 0x4000 && address <= 0x43FF) {
        ram[address] = value;
    } else {
        Logger::warn("Attempted write to undefined memory address: {:X}", address);
    }
}

u8& MMU::getRef(u16 address) {
    if (address >= 0x0 && address <= 0x1FFF) {
        return rom[address];
    } else if (address >= 0x2000 && address <= 0x23FF) {
        return ram[address];
    } else if (address >= 0x2400 && address <= 0x3FFF) {
        return vram[address];
    } else if (address >= 0x4000 && address <= 0x43FF) {
        return ram[address];
    }
    // TODO: handle illegal reference
}

const std::array<u8, 1024 * 7>& MMU::getFrameBuffer() const {
    return vram;
};