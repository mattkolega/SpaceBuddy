#include "mmu.h"

#include <algorithm>

#include "common/crc.h"
#include "common/log.h"

bool MMU::loadBufferIntoRom(const std::vector<u8>& buffer) {
    if (buffer.size() > ROM_SIZE) {
        log::fatal("Buffer exceeds ROM size");
        return false;
    }

    static constexpr u32 ROM_CHECKSUM {0xB64CA815}; // CRC32 for Space Invaders ROM
    if (crc::crc32(buffer) != ROM_CHECKSUM) {
        log::fatal("Invalid ROM provided");
        return false;
    }

    std::copy(buffer.begin(), buffer.end(), m_rom.begin());

    return true;
}

u8 MMU::read(u16 address) const {
    if (address <= 0x1FFF) {
        return m_rom[address];
    } else if (address <= 0x23FF) {
        return m_ram[address - 0x2000];
    } else if (address <= 0x3FFF) {
        return m_vram[address - 0x2400];
    } else if (address <= 0x43FF) {
        return m_ram[address - 0x4000];
    } else if (address <= 0x5FFF) {
        return m_vram[address - 0x4400];
    } else {
        log::warn("Attempted read from unmapped memory at address: {:X}", address);
        return 0;
    }
}

void MMU::write(u16 address, u8 value) {
    if (address <= 0x1FFF) {
        log::warn("Illegal write to ROM at address: {:X}", address);
        return;
    } else if (address <= 0x23FF) {
        m_ram[address - 0x2000] = value;
    } else if (address <= 0x3FFF) {
        m_vram[address - 0x2400] = value;
    } else if (address <= 0x43FF) {
        m_ram[address - 0x4000] = value;
    } else if (address <= 0x5FFF) {
        m_vram[address - 0x4400] = value;
    } else {
        log::warn("Illegal write to unmapped memory at address: {:X}", address);
        return;
    }
}

std::span<const u8, MMU::VRAM_SIZE> MMU::getVram() const {
    return m_vram;
};