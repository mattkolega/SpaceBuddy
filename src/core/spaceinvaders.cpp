#include "spaceinvaders.h"

#include <utility>

#include "common/bits.h"
#include "common/fs.h"

u8 ShiftRegister::read() const {
    return static_cast<u8>((m_value << m_offset) >> 8);
}

void ShiftRegister::writeOffset(u8 offset) {
    m_offset = offset & 0b111;
}

void ShiftRegister::writeValue(u8 value) {
    m_value >>= 8;
    m_value |= static_cast<u16>(value) << 8;
}

void IOPorts::handlePlayerInput(InputType inputType, bool pressed) {
    u8 portNum = bits::getFirstNibble(std::to_underlying(inputType));
    u8 bit     = bits::getSecondNibble(std::to_underlying(inputType));

    if (portNum == 1) {
        m_in1 = bits::modifyBitInByte(m_in1, bit, pressed ? 1 : 0);
    } else if (portNum == 2) {
        m_in2 = bits::modifyBitInByte(m_in2, bit, pressed ? 1 : 0);
    }
}

u8 IOPorts::in(u8 portNum) const {
    switch (portNum) {
    case 1:
        return m_in1;
    case 2:
        return m_in2;
    case 3:
        return m_shiftRegister.read();
    default:
        return 0;
    }
}

void IOPorts::out(u8 portNum, u8 value) {
    switch (portNum) {
    case 2:
        m_shiftRegister.writeOffset(value);
        break;
    case 3:
        m_out3 = value;
        break;
    case 4:
        m_shiftRegister.writeValue(value);
        break;
    case 5:
        m_out5 = value;
        break;
    }
}

bool SpaceInvaders::loadRom(std::string_view romPath) {
    auto buffer = fs::loadFileIntoBuffer(romPath);
    if (!buffer) return false;

    bool success = m_mmu.loadBufferIntoRom(*buffer);
    if (!success) return false;

    return true;
}

void SpaceInvaders::runFrame() {
    for (usize i{0}; i < (CYCLES_PER_SECOND / REFRESH_RATE / 2); i++) {
        m_cpu.step();
    }
    m_cpu.triggerInterrupt(InterruptType::MidFrame);
    for (usize i{0}; i < (CYCLES_PER_SECOND / REFRESH_RATE / 2); i++) {
        m_cpu.step();
    }
    m_cpu.triggerInterrupt(InterruptType::EndFrame);
}

std::span<const u32, SpaceInvaders::FRAMEBUFFER_WIDTH * SpaceInvaders::FRAMEBUFFER_HEIGHT>
SpaceInvaders::getFramebuffer() {
    auto vram = m_mmu.getVram();

    for (int x{0}; x < FRAMEBUFFER_WIDTH; x++) {
        for (int y{0}; y < FRAMEBUFFER_HEIGHT; y++) {
            u16 addr = (x * 32) + (y / 8);
            u8 bit = y % 8;
            bool pixel = (vram[addr] >> bit) & 1;

            int screenX = x;
            int screenY = 255 - y;
            m_framebuffer[screenY * 224 + screenX] = pixel ? 0xFFFFFFFF : 0x00000000;
        }
    }

    return m_framebuffer;
}

void SpaceInvaders::handleInput(InputType inputType, bool pressed) {
    m_ports.handlePlayerInput(inputType, pressed);
}