#include "mmu.h"

#include <fstream>
#include <stdexcept>

#include <common/dialog.h>
#include <common/logger.h>
#include <common/types.h>

void MMU::loadRom() {
    auto filepath = Dialog::openFile("Open Space Invaders ROM File", {"*.rom"}, "Space Invaders ROM");
    if (filepath.empty()) throw std::runtime_error("ROM file is empty");

    std::ifstream romFile(filepath, std::ios::binary);
    if (!romFile) throw std::runtime_error("Failed to open ROM file: " + filepath);

    Logger::info("{} {}", "Loaded ROM: ", filepath);

    romFile.seekg(0, std::ios::end);
    auto fileSize = romFile.tellg();
    romFile.seekg(0, std::ios::beg);

    if (fileSize > 8192) throw std::runtime_error("ROM size is too big.");

    if (!romFile.read(reinterpret_cast<char*>(rom.data()), fileSize)) {
        throw std::runtime_error("Failed to read the ROM file.");
    }
}

u8 MMU::read(u16 address) const {
    if (address <= 0x1FFF) {
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
    if (address <= 0x1FFF) {
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
    if (address <= 0x1FFF) {
        return rom[address];
    } else if (address >= 0x2000 && address <= 0x23FF) {
        return ram[address];
    } else if (address >= 0x2400 && address <= 0x3FFF) {
        return vram[address];
    } else if (address >= 0x4000 && address <= 0x43FF) {
        return ram[address];
    } else {
        throw std::runtime_error("Illegal reference to undefined memory address: " + std::to_string(address));
    }
}

const std::array<u8, 1024 * 7>& MMU::getFrameBuffer() const {
    return vram;
};