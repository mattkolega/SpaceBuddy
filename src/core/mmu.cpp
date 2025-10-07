#include "mmu.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include <common/dialog.h>
#include <common/logger.h>
#include <common/types.h>

template<size_t N>
static void loadRomFile(const std::string& filepath, std::array<u8, N>& dest) {
    std::ifstream romFile(filepath, std::ios::binary);
    if (!romFile) throw std::runtime_error("Failed to open ROM file: " + std::filesystem::absolute(filepath).string());

    Logger::info("{} {}", "Loaded ROM: ", std::filesystem::absolute(filepath).string());

    romFile.seekg(0, std::ios::end);
    auto fileSize = romFile.tellg();
    romFile.seekg(0, std::ios::beg);

    if (fileSize > dest.size()) throw std::runtime_error("ROM size is too big.");

    if (!romFile.read(reinterpret_cast<char*>(dest.data()), fileSize)) {
        throw std::runtime_error("Failed to read the ROM file.");
    }
}

void RealMMU::loadRom() {
    auto filepath = Dialog::openFile("Open Space Invaders ROM File", {"*.rom"}, "Space Invaders ROM");
    if (filepath.empty()) throw std::runtime_error("ROM file is empty");

    loadRomFile(filepath, rom);
}

u8 RealMMU::read(u16 address) const {
    if (address <= 0x1FFF) {
        return rom[address];
    } else if (address <= 0x23FF) {
        return ram[address - 0x2000];
    } else if (address <= 0x3FFF) {
        return vram[address - 0x2400];
    } else if (address <= 0x43FF) {
        return ram[address - 0x4000];
    } else {
        Logger::warn("Read from undefined memory address: {:X}", address);
        return 0;
    }
}

void RealMMU::write(u16 address, u8 value) {
    if (address <= 0x1FFF) {
        Logger::warn("Illegal write to ROM at address: {:X}", address);
    } else if (address <= 0x23FF) {
        ram[address - 0x2000] = value;
    } else if (address <= 0x3FFF) {
        vram[address - 0x2400] = value;
    } else if (address <= 0x43FF) {
        ram[address - 0x4000] = value;
    } else {
        Logger::warn("Attempted write to undefined memory address: {:X}", address);
    }
}

u8& RealMMU::getRef(u16 address) {
    if (address <= 0x1FFF) {
        return rom[address];
    } else if (address <= 0x23FF) {
        return ram[address - 0x2000];
    } else if (address <= 0x3FFF) {
        return vram[address - 0x2400];
    } else if (address <= 0x43FF) {
        return ram[address - 0x4000];
    } else {
        throw std::runtime_error("Illegal reference to undefined memory address: " + std::to_string(address));
    }
}

std::span<const u8> RealMMU::getFrameBuffer() const {
    return vram;
};

void TestMMU::loadRom() {
    loadRomFile("cpudiag.bin", memory);
}

u8 TestMMU::read(u16 address) const {
    return memory[address];
}

void TestMMU::write(u16 address, u8 value) {
    memory[address] = value;
}

u8& TestMMU::getRef(u16 address) {
    return memory[address];
}

std::span<const u8> TestMMU::getFrameBuffer() const {
    return {};
};