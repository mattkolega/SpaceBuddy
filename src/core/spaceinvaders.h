#pragma once

#include <string_view>

#include "common/types.h"
#include "cpu.h"
#include "mmu.h"

// Contains all possible player inputs.
// First nibble is the port number.
// Second nibble is the bit corresponding to the input.
enum class InputType : u8 {
    // Port 1
    Credit  = 0x10,
    P2Start = 0x11,
    P1Start = 0x12,
    P1Fire  = 0x14,
    P1Left  = 0x15,
    P1Right = 0x16,
    // Port 2
    P2Fire  = 0x24,
    P2Left  = 0x25,
    P2Right = 0x26,
};

// Assists with the rendering of sprites to the display.
// Compensates for the lack of bit-shifting instructions in the CPU.
class ShiftRegister {
public:
    u8 read() const;
    void writeOffset(u8 offset);
    void writeValue(u8 value);
private:
    u16 m_value  {};
    u8  m_offset {};
};

// Handles reading and writing to I/O ports.
// These are mapped to player controls, audio circuitry and the shift register.
class IOPorts {
public:
    IOPorts(ShiftRegister& shiftRegister) : m_shiftRegister(shiftRegister) {}

    void handlePlayerInput(InputType input, bool pressed);

    // Used by CPU to read from input ports
    u8 in(u8 portNum) const;

    // Used by CPU to write to outport ports
    void out(u8 portNum, u8 value);
private:
    ShiftRegister& m_shiftRegister;

    u8 m_in1 {}, m_in2 {}, m_out3 {}, m_out5 {};
};

class SpaceInvaders {
public:
    static constexpr int FRAMEBUFFER_WIDTH  {224};
    static constexpr int FRAMEBUFFER_HEIGHT {256};
    static constexpr int CYCLES_PER_SECOND  {2'000'000};
    static constexpr int REFRESH_RATE       {60};

    SpaceInvaders() : m_ports(m_shiftreg), m_cpu(m_mmu, m_ports) {};

    SpaceInvaders(const SpaceInvaders&) = delete;
    SpaceInvaders& operator=(const SpaceInvaders&) = delete;

    SpaceInvaders(SpaceInvaders&& other)
        : m_mmu(std::move(other.m_mmu))
        , m_shiftreg(std::move(other.m_shiftreg))
        , m_ports(m_shiftreg)
        , m_cpu(m_mmu, m_ports) {};
    SpaceInvaders& operator=(SpaceInvaders&& other) = delete;

    bool loadRom(std::string_view romPath);

    void runFrame();

    std::span<const u32, FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT> getFramebuffer();

    void handleInput(InputType input, bool pressed);

private:
    MMU           m_mmu;
    ShiftRegister m_shiftreg;
    IOPorts       m_ports;
    CPU           m_cpu;

    std::array<u32, FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT> m_framebuffer;
};