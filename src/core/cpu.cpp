#include "cpu.h"

#include <utility>

#include "common/bits.h"
#include "common/types.h"

#include "spaceinvaders.h"
#include "mmu.h"

// Make sure we're running on a little-endian system, CPU won't work properly if not
static_assert(std::endian::native == std::endian::little,
    "Host system must be little-endian");

static constexpr std::array<OpcodeInfo, 256> opcodeInfoTable {{
    {"NOP", 4},          // 0x00
    {"LXI BC, d16", 10}, // 0x01
    {"STAX BC", 7},      // 0x02
    {"INX BC", 5},       // 0x03
    {"INR B", 5},        // 0x04
    {"DCR B", 5},        // 0x05
    {"MVI B, d8", 7},    // 0x06
    {"RLC", 4},          // 0x07
    {"NOP", 4},          // 0x08
    {"DAD BC", 10},      // 0x09
    {"LDAX BC", 7},      // 0x0A
    {"DCX BC", 5},       // 0x0B
    {"INR C", 5},        // 0x0C
    {"DCR C", 5},        // 0x0D
    {"MVI C, d8", 7},    // 0x0E
    {"RRC", 4},          // 0x0F

    {"NOP", 4},          // 0x10
    {"LXI DE, d16", 10}, // 0x11
    {"STAX DE", 7},      // 0x12
    {"INX DE", 5},       // 0x13
    {"INR D", 5},        // 0x14
    {"DCR D", 5},        // 0x15
    {"MVI D, d8", 7},    // 0x16
    {"RAL", 4},          // 0x17
    {"NOP", 4},          // 0x18
    {"DAD DE", 10},      // 0x19
    {"LDAX DE", 7},      // 0x1A
    {"DCX DE", 5},       // 0x1B
    {"INR E", 5},        // 0x1C
    {"DCR E", 5},        // 0x1D
    {"MVI E, d8", 7},    // 0x1E
    {"RAR", 4},          // 0x1F

    {"NOP", 4},          // 0x20
    {"LXI HL, d16", 10}, // 0x21
    {"SHLD a16", 16},    // 0x22
    {"INX HL", 5},       // 0x23
    {"INR H", 5},        // 0x24
    {"DCR H", 5},        // 0x25
    {"MVI H, d8", 7},    // 0x26
    {"DAA", 4},          // 0x27
    {"NOP", 4},          // 0x28
    {"DAD HL", 10},      // 0x29
    {"LHLD a16", 16},    // 0x2A
    {"DCX HL", 5},       // 0x2B
    {"INR L", 5},        // 0x2C
    {"DCR L", 5},        // 0x2D
    {"MVI L, d8", 7},    // 0x2E
    {"CMA", 4},          // 0x2F

    {"NOP", 4},          // 0x30
    {"LXI SP, d16", 10}, // 0x31
    {"STA a16", 13},     // 0x32
    {"INX SP", 5},       // 0x33
    {"INR M", 10},       // 0x34
    {"DCR M", 10},       // 0x35
    {"MVI M, d8", 10},   // 0x36
    {"STC", 4},          // 0x37
    {"NOP", 4},          // 0x38
    {"DAD SP", 10},      // 0x39
    {"LDA a16", 13},     // 0x3A
    {"DCX SP", 5},       // 0x3B
    {"INR A", 5},        // 0x3C
    {"DCR A", 5},        // 0x3D
    {"MVI A, d8", 7},    // 0x3E
    {"CMC", 4},          // 0x3F

    {"MOV B,B", 5},      // 0x40
    {"MOV B,C", 5},      // 0x41
    {"MOV B,D", 5},      // 0x42
    {"MOV B,E", 5},      // 0x43
    {"MOV B,H", 5},      // 0x44
    {"MOV B,L", 5},      // 0x45
    {"MOV B,M", 7},      // 0x46
    {"MOV B,A", 5},      // 0x47
    {"MOV C,B", 5},      // 0x48
    {"MOV C,C", 5},      // 0x49
    {"MOV C,D", 5},      // 0x4A
    {"MOV C,E", 5},      // 0x4B
    {"MOV C,H", 5},      // 0x4C
    {"MOV C,L", 5},      // 0x4D
    {"MOV C,M", 7},      // 0x4E
    {"MOV C,A", 5},      // 0x4F

    {"MOV D,B", 5},      // 0x50
    {"MOV D,C", 5},      // 0x51
    {"MOV D,D", 5},      // 0x52
    {"MOV D,E", 5},      // 0x53
    {"MOV D,H", 5},      // 0x54
    {"MOV D,L", 5},      // 0x55
    {"MOV D,M", 7},      // 0x56
    {"MOV D,A", 5},      // 0x57
    {"MOV E,B", 5},      // 0x58
    {"MOV E,C", 5},      // 0x59
    {"MOV E,D", 5},      // 0x5A
    {"MOV E,E", 5},      // 0x5B
    {"MOV E,H", 5},      // 0x5C
    {"MOV E,L", 5},      // 0x5D
    {"MOV E,M", 7},      // 0x5E
    {"MOV E,A", 5},      // 0x5F

    {"MOV H,B", 5},      // 0x60
    {"MOV H,C", 5},      // 0x61
    {"MOV H,D", 5},      // 0x62
    {"MOV H,E", 5},      // 0x63
    {"MOV H,H", 5},      // 0x64
    {"MOV H,L", 5},      // 0x65
    {"MOV H,M", 7},      // 0x66
    {"MOV H,A", 5},      // 0x67
    {"MOV L,B", 5},      // 0x68
    {"MOV L,C", 5},      // 0x69
    {"MOV L,D", 5},      // 0x6A
    {"MOV L,E", 5},      // 0x6B
    {"MOV L,H", 5},      // 0x6C
    {"MOV L,L", 5},      // 0x6D
    {"MOV L,M", 7},      // 0x6E
    {"MOV L,A", 5},      // 0x6F

    {"MOV M,B", 7},      // 0x70
    {"MOV M,C", 7},      // 0x71
    {"MOV M,D", 7},      // 0x72
    {"MOV M,E", 7},      // 0x73
    {"MOV M,H", 7},      // 0x74
    {"MOV M,L", 7},      // 0x75
    {"HLT", 7},          // 0x76
    {"MOV M,A", 7},      // 0x77
    {"MOV A,B", 5},      // 0x78
    {"MOV A,C", 5},      // 0x79
    {"MOV A,D", 5},      // 0x7A
    {"MOV A,E", 5},      // 0x7B
    {"MOV A,H", 5},      // 0x7C
    {"MOV A,L", 5},      // 0x7D
    {"MOV A,M", 7},      // 0x7E
    {"MOV A,A", 5},      // 0x7F

    {"ADD B", 4},        // 0x80
    {"ADD C", 4},        // 0x81
    {"ADD D", 4},        // 0x82
    {"ADD E", 4},        // 0x83
    {"ADD H", 4},        // 0x84
    {"ADD L", 4},        // 0x85
    {"ADD M", 7},        // 0x86
    {"ADD A", 4},        // 0x87
    {"ADC B", 4},        // 0x88
    {"ADC C", 4},        // 0x89
    {"ADC D", 4},        // 0x8A
    {"ADC E", 4},        // 0x8B
    {"ADC H", 4},        // 0x8C
    {"ADC L", 4},        // 0x8D
    {"ADC M", 7},        // 0x8E
    {"ADC A", 4},        // 0x8F

    {"SUB B", 4},        // 0x90
    {"SUB C", 4},        // 0x91
    {"SUB D", 4},        // 0x92
    {"SUB E", 4},        // 0x93
    {"SUB H", 4},        // 0x94
    {"SUB L", 4},        // 0x95
    {"SUB M", 7},        // 0x96
    {"SUB A", 4},        // 0x97
    {"SBB B", 4},        // 0x98
    {"SBB C", 4},        // 0x99
    {"SBB D", 4},        // 0x9A
    {"SBB E", 4},        // 0x9B
    {"SBB H", 4},        // 0x9C
    {"SBB L", 4},        // 0x9D
    {"SBB M", 7},        // 0x9E
    {"SBB A", 4},        // 0x9F

    {"ANA B", 4},        // 0xA0
    {"ANA C", 4},        // 0xA1
    {"ANA D", 4},        // 0xA2
    {"ANA E", 4},        // 0xA3
    {"ANA H", 4},        // 0xA4
    {"ANA L", 4},        // 0xA5
    {"ANA M", 7},        // 0xA6
    {"ANA A", 4},        // 0xA7
    {"XRA B", 4},        // 0xA8
    {"XRA C", 4},        // 0xA9
    {"XRA D", 4},        // 0xAA
    {"XRA E", 4},        // 0xAB
    {"XRA H", 4},        // 0xAC
    {"XRA L", 4},        // 0xAD
    {"XRA M", 7},        // 0xAE
    {"XRA A", 4},        // 0xAF

    {"ORA B", 4},        // 0xB0
    {"ORA C", 4},        // 0xB1
    {"ORA D", 4},        // 0xB2
    {"ORA E", 4},        // 0xB3
    {"ORA H", 4},        // 0xB4
    {"ORA L", 4},        // 0xB5
    {"ORA M", 7},        // 0xB6
    {"ORA A", 4},        // 0xB7
    {"CMP B", 4},        // 0xB8
    {"CMP C", 4},        // 0xB9
    {"CMP D", 4},        // 0xBA
    {"CMP E", 4},        // 0xBB
    {"CMP H", 4},        // 0xBC
    {"CMP L", 4},        // 0xBD
    {"CMP M", 7},        // 0xBE
    {"CMP A", 4},        // 0xBF

    {"RNZ", 5},          // 0xC0
    {"POP BC", 10},      // 0xC1
    {"JNZ a16", 10},     // 0xC2
    {"JMP a16", 10},     // 0xC3
    {"CNZ a16", 11},     // 0xC4
    {"PUSH BC", 11},     // 0xC5
    {"ADI d8", 7},       // 0xC6
    {"RST 0", 11},       // 0xC7
    {"RZ", 5},           // 0xC8
    {"RET", 10},         // 0xC9
    {"JZ a16", 10},      // 0xCA
    {"JMP a16", 10},     // 0xCB
    {"CZ a16", 11},      // 0xCC
    {"CALL a16", 11},    // 0xCD
    {"ACI d8", 7},       // 0xCE
    {"RST 1", 11},       // 0xCF

    {"RNC", 5},          // 0xD0
    {"POP DE", 10},      // 0xD1
    {"JNC a16", 10},     // 0xD2
    {"OUT d8", 10},      // 0xD3
    {"CNC a16", 11},     // 0xD4
    {"PUSH DE", 11},     // 0xD5
    {"SUI d8", 7},       // 0xD6
    {"RST 2", 11},       // 0xD7
    {"RC", 5},           // 0xD8
    {"RET", 10},         // 0xD9
    {"JC a16", 10},      // 0xDA
    {"IN d8", 10},       // 0xDB
    {"CC a16", 11},      // 0xDC
    {"CALL a16", 11},    // 0xDD
    {"SBI d8", 7},       // 0xDE
    {"RST 3", 11},       // 0xDF

    {"RPO", 5},          // 0xE0
    {"POP HL", 10},      // 0xE1
    {"JPO a16", 10},     // 0xE2
    {"XTHL", 18},        // 0xE3
    {"CPO a16", 11},     // 0xE4
    {"PUSH HL", 11},     // 0xE5
    {"ANI d8", 7},       // 0xE6
    {"RST 4", 11},       // 0xE7
    {"RPE", 5},          // 0xE8
    {"PCHL", 5},         // 0xE9
    {"JPE a16", 10},     // 0xEA
    {"XCHG", 5},         // 0xEB
    {"CPE a16", 11},     // 0xEC
    {"CALL a16", 11},    // 0xED
    {"XRI d8", 7},       // 0xEE
    {"RST 5", 11},       // 0xEF

    {"RP", 5},           // 0xF0
    {"POP PSW", 10},     // 0xF1
    {"JP a16", 10},      // 0xF2
    {"DI", 4},           // 0xF3
    {"CP a16", 11},      // 0xF4
    {"PUSH PSW", 11},    // 0xF5
    {"ORI d8", 7},       // 0xF6
    {"RST 6", 11},       // 0xF7
    {"RM", 5},           // 0xF8
    {"SPHL", 5},         // 0xF9
    {"JM a16", 10},      // 0xFA
    {"EI", 4},           // 0xFB
    {"CM a16", 11},      // 0xFC
    {"CALL a16", 11},    // 0xFD
    {"CPI d8", 7},       // 0xFE
    {"RST 7", 11},       // 0xFF
}};

void CPU::step() {
    if (!cycleDelay) {
        u8 opcode {};

        if (m_midFrameInterrupt && m_interruptsEnabled) {
            opcode = 0xCF;
            m_isHalted = false;
            m_midFrameInterrupt = false;
            m_interruptsEnabled = false;
        } else if (m_endFrameInterrupt && m_interruptsEnabled) {
            opcode = 0xD7;
            m_isHalted = false;
            m_endFrameInterrupt = false;
            m_interruptsEnabled = false;
        } else {
            if (m_isHalted) return;
            opcode = m_mmu.read(m_pc);
            m_pc++;
        }

        execute(opcode);
        auto opcodeInfo = opcodeInfoTable[opcode];
        cycleDelay += opcodeInfo.cycles;
    }
    cycleDelay--;
}

void CPU::triggerInterrupt(InterruptType interruptType) {
    using enum InterruptType;

    if (interruptType == MidFrame) {
        m_midFrameInterrupt = true;
    } else if (interruptType == EndFrame) {
        m_endFrameInterrupt = true;
    }
}

u8 CPU::readReg8(Reg8 reg) {
    using enum Reg8;
    switch (reg) {
    case A:
        return m_af.hi;
    case F:
        return m_af.lo;
    case B:
        return m_bc.hi;
    case C:
        return m_bc.lo;
    case D:
        return m_de.hi;
    case E:
        return m_de.lo;
    case H:
        return m_hl.hi;
    case L:
        return m_hl.lo;
    case M:
        return m_mmu.read(m_hl.full);
    }
}

u16 CPU::readReg16(Reg16 reg) {
    using enum Reg16;
    switch (reg) {
    case AF:
        return m_af.full;
    case BC:
        return m_bc.full;
    case DE:
        return m_de.full;
    case HL:
        return m_hl.full;
    case SP:
        return m_sp;
    }
}

void CPU::writeReg8(Reg8 reg, u8 value) {
    using enum Reg8;
    switch (reg) {
    case A:
        m_af.hi = value;
        break;
    case F:
        m_af.lo = value;
        break;
    case B:
        m_bc.hi = value;
        break;
    case C:
        m_bc.lo = value;
        break;
    case D:
        m_de.hi = value;
        break;
    case E:
        m_de.lo = value;
        break;
    case H:
        m_hl.hi = value;
        break;
    case L:
        m_hl.lo = value;
        break;
    case M:
        m_mmu.write(m_hl.full, value);
        break;
    }
}

void CPU::writeReg16(Reg16 reg, u16 value) {
    using enum Reg16;
    switch (reg) {
    case AF:
        m_af.full = value;
        break;
    case BC:
        m_bc.full = value;
        break;
    case DE:
        m_de.full = value;
        break;
    case HL:
        m_hl.full = value;
        break;
    case SP:
        m_sp = value;
        break;
    }
}

void CPU::pushToStack(u16 value) {
    m_sp--;
    m_mmu.write(m_sp, (value >> 8) & 0xFF);
    m_sp--;
    m_mmu.write(m_sp, value & 0xFF);
}

u16 CPU::popFromStack() {
    u8 lo = m_mmu.read(m_sp);
    m_sp++;
    u8 hi = m_mmu.read(m_sp);
    m_sp++;
    return bits::concatBytes(lo, hi);
}

u8 CPU::getCarry() const    { return bits::getBitInByte(m_af.lo, std::to_underlying(CPUFlags::C)); }
u8 CPU::getParity() const   { return bits::getBitInByte(m_af.lo, std::to_underlying(CPUFlags::P)); }
u8 CPU::getAuxCarry() const { return bits::getBitInByte(m_af.lo, std::to_underlying(CPUFlags::A)); }
u8 CPU::getZero() const     { return bits::getBitInByte(m_af.lo, std::to_underlying(CPUFlags::Z)); }
u8 CPU::getSign() const     { return bits::getBitInByte(m_af.lo, std::to_underlying(CPUFlags::S)); }

void CPU::setCarry(bool value)    { m_af.lo = bits::modifyBitInByte(m_af.lo, std::to_underlying(CPUFlags::C), value); }
void CPU::setParity(bool value)   { m_af.lo = bits::modifyBitInByte(m_af.lo, std::to_underlying(CPUFlags::P), value); }
void CPU::setAuxCarry(bool value) { m_af.lo = bits::modifyBitInByte(m_af.lo, std::to_underlying(CPUFlags::A), value); }
void CPU::setZero(bool value)     { m_af.lo = bits::modifyBitInByte(m_af.lo, std::to_underlying(CPUFlags::Z), value); }
void CPU::setSign(bool value)     { m_af.lo = bits::modifyBitInByte(m_af.lo, std::to_underlying(CPUFlags::S), value); }

/* INSTRUCTIONS */

// Carry bit instructions

// Complements carry flag
void CPU::cmc() {
    setCarry(!getCarry());
}

// Sets carry flag
void CPU::stc() {
    setCarry(1);
}

// Single register instructions

// Increments register
void CPU::inr(Reg8 reg) {
    u8 originalValue = readReg8(reg);
    u8 newValue = originalValue + 1;

    setZero(newValue == 0);
    setSign(bits::isNegative(newValue));
    setParity(bits::checkParity(newValue));
    setAuxCarry(bits::checkHalfCarryAdd(originalValue, 1));

    writeReg8(reg, newValue);
}

// Decrements register
void CPU::dcr(Reg8 reg) {
    u8 originalValue = readReg8(reg);
    u8 newValue = originalValue - 1;

    setZero(newValue == 0);
    setSign(bits::isNegative(newValue));
    setParity(bits::checkParity(newValue));
    setAuxCarry(bits::checkHalfCarrySub(originalValue, 1));

    writeReg8(reg, newValue);
}

// Complements accumulator
void CPU::cma() {
    writeReg8(Reg8::A, ~readReg8(Reg8::A));
}

// Decimal adjusts accumulator
void CPU::daa() {
    u8 offset { 0 };

    u8 accumulator = readReg8(Reg8::A);

    if ((accumulator & 0xF) > 0x9 || getAuxCarry() == 1) {
        offset |= 0x06;
        setAuxCarry(1);
    } else {
        setAuxCarry(0);
    }

    if (accumulator > 0x99 || getCarry() == 1) {
        offset |= 0x60;
        setCarry(1);
    } else {
        setCarry(0);
    }

    accumulator += offset;

    // Set flags
    setZero(accumulator == 0);
    setSign(bits::isNegative(accumulator));
    setParity(bits::checkParity(accumulator));

    writeReg8(Reg8::A, accumulator);
}

// Data transfer instructions

// Moves data to register
void CPU::mov(Reg8 dst, u8 src) {
    writeReg8(dst, src);
}

// Stores accumulator at memory address provided by BC or DE
void CPU::stax(Reg16 reg) {
    m_mmu.write(readReg16(reg), readReg8(Reg8::A));
}

// Loads accumulator with memory value at address provided by BC or DE
void CPU::ldax(Reg16 reg) {
    writeReg8(Reg8::A, m_mmu.read(readReg16(reg)));
}

// Arithmetic instructions

// Adds value to accumulator
void CPU::add(u8 value) {
    u8 originalValue = readReg8(Reg8::A);
    u8 newValue = originalValue + value;

    setZero(newValue == 0);
    setSign(bits::isNegative(newValue));
    setParity(bits::checkParity(newValue));
    setAuxCarry(bits::checkHalfCarryAdd(originalValue, value));
    setCarry((originalValue + value) > 0xFF);

    writeReg8(Reg8::A, newValue);
};

// Adds value + carry to accumulator
void CPU::adc(u8 value) {
    u8 originalValue = readReg8(Reg8::A);
    u8 newValue = originalValue + value + getCarry();

    setZero(newValue == 0);
    setSign(bits::isNegative(newValue));
    setParity(bits::checkParity(newValue));
    setAuxCarry(bits::checkHalfCarryAdd(originalValue, value, getCarry()));
    setCarry((originalValue + value + getCarry()) > 0xFF);

    writeReg8(Reg8::A, newValue);
};

// Subtracts value from accumulator
void CPU::sub(u8 value) {
    u8 originalValue = readReg8(Reg8::A);
    u8 newValue = originalValue - value;

    setZero(newValue == 0);
    setSign(bits::isNegative(newValue));
    setParity(bits::checkParity(newValue));
    setAuxCarry(bits::checkHalfCarrySub(originalValue, value));
    setCarry((originalValue - value) < 0);

    writeReg8(Reg8::A, newValue);
};

// Subtracts value + carry from accumulator
void CPU::sbb(u8 value) {
    u8 originalValue = readReg8(Reg8::A);
    u8 newValue = originalValue - value - getCarry();

    setZero(newValue == 0);
    setSign(bits::isNegative(newValue));
    setParity(bits::checkParity(newValue));
    setAuxCarry(bits::checkHalfCarrySub(originalValue, value, getCarry()));
    setCarry((originalValue - value - getCarry()) < 0);

    writeReg8(Reg8::A, newValue);
};

// Logical instructions

// ANDs value with accumulator
void CPU::ana(u8 value) {
    u8 newValue = readReg8(Reg8::A) & value;

    setZero(newValue == 0);
    setSign(bits::isNegative(newValue));
    setParity(bits::checkParity(newValue));
    setCarry(0);

    writeReg8(Reg8::A, newValue);
}

// XORs value with accumulator
void CPU::xra(u8 value) {
    u8 newValue = readReg8(Reg8::A) ^ value;

    setZero(newValue == 0);
    setSign(bits::isNegative(newValue));
    setParity(bits::checkParity(newValue));
    setAuxCarry(0);
    setCarry(0);

    writeReg8(Reg8::A, newValue);
}

// ORs value with accumulator
void CPU::ora(u8 value) {
    u8 newValue = readReg8(Reg8::A) | value;

    setZero(newValue == 0);
    setSign(bits::isNegative(newValue));
    setParity(bits::checkParity(newValue));
    setAuxCarry(0);
    setCarry(0);

    writeReg8(Reg8::A, newValue);
}

// Compares value to accumulator
void CPU::cmp(u8 value) {
    u8 accumulator = readReg8(Reg8::A);
    u8 result = accumulator - value;

    setZero(accumulator == value);
    setSign(bits::isNegative(result));
    setParity(bits::checkParity(result));
    setAuxCarry(bits::checkHalfCarrySub(accumulator, value));
    setCarry(accumulator < value);
}

// Rotation instructions

// Rotate accumulator left
void CPU::rlc() {
    u8 accumulator = readReg8(Reg8::A);
    setCarry(bits::getBitInByte(accumulator, 7)); // Set carry to leftmost bit
    writeReg8(Reg8::A, std::rotl(accumulator, 1));
}

// Rotate accumulator right
void CPU::rrc() {
    u8 accumulator = readReg8(Reg8::A);
    setCarry(bits::getBitInByte(accumulator, 0)); // Set carry to rightmost bit
    writeReg8(Reg8::A, std::rotr(accumulator, 1));
}

// Rotate accumulator left through carry
void CPU::ral() {
    u8 carry = getCarry();
    u8 accumulator = readReg8(Reg8::A);
    setCarry(bits::getBitInByte(accumulator, 7)); // Set carry flag to leftmost bit
    u8 rotated = std::rotl(accumulator, 1);
    writeReg8(Reg8::A, bits::modifyBitInByte(rotated, 0, carry));
}

// Rotate accumulator right through carry
void CPU::rar() {
    u8 carry = getCarry();
    u8 accumulator = readReg8(Reg8::A);
    setCarry(bits::getBitInByte(accumulator, 0));  // Set carry flag to rightmost bit
    u8 rotated = std::rotr(accumulator, 1);
    writeReg8(Reg8::A, bits::modifyBitInByte(rotated, 7, carry));
}

// Register pair instructions

// Pushes register pair to stack
void CPU::push(Reg16 reg) {
    pushToStack(readReg16(reg));
}

// Pops register pair from stack
void CPU::pop(Reg16 reg) {
    writeReg16(reg, popFromStack());
}

// Adds register pair to HL
void CPU::dad(Reg16 reg) {
    u16 originalValue = readReg16(Reg16::HL);
    u16 registerValue = readReg16(reg);
    u16 newValue = originalValue + registerValue;

    setCarry((originalValue + registerValue) > 0xFFFF);

    writeReg16(Reg16::HL, newValue);
}

// Increments word
void CPU::inx(Reg16 reg) {
    writeReg16(reg, readReg16(reg)+1);
}

// Decrements word
void CPU::dcx(Reg16 reg) {
    writeReg16(reg, readReg16(reg)-1);
}

// Swaps DE and HL values
void CPU::xchg() {
    std::swap(m_de.full, m_hl.full);
}

// Swaps HL and top word on stack
void CPU::xthl(){
    u8 lo = m_mmu.read(m_sp);
    m_mmu.write(m_sp, readReg8(Reg8::L));
    u8 hi = m_mmu.read(m_sp+1);
    m_mmu.write(m_sp+1, readReg8(Reg8::H));
    writeReg16(Reg16::HL, bits::concatBytes(lo, hi));
}

// Sets SP to HL
void CPU::sphl() {
    m_sp = m_hl.full;
}

// Immediate instructions

// Loads immediate value into word-sized register
void CPU::lxi(Reg16 reg, u16 immediate) {
    writeReg16(reg, immediate);
}

// Direct addressing instructions

// Stores accumulator in memory
void CPU::sta(u16 address) {
    m_mmu.write(address, readReg8(Reg8::A));
}

// Loads accumulator from memory
void CPU::lda(u16 address) {
    writeReg8(Reg8::A, m_mmu.read(address));
}

// Stores HL in memory
void CPU::shld(u16 address) {
    m_mmu.write(address, readReg8(Reg8::L));
    m_mmu.write(address+1, readReg8(Reg8::H));
}

// Loads HL from memory
void CPU::lhld(u16 address) {
    writeReg8(Reg8::L, m_mmu.read(address));
    writeReg8(Reg8::H, m_mmu.read(address+1));
}

// Jump instructions

// Jumps to address in HL
void CPU::pchl() {
    m_pc = readReg16(Reg16::HL);
}

// Jumps to address
void CPU::jmp(bool condition = true) {
    if (condition) {
        m_pc = bits::concatBytes(m_mmu.read(m_pc), m_mmu.read(m_pc+1));
    } else {
        m_pc += 2;
    }
}

// Call subroutine instructions

// Pushes PC to stack and jumps to address
void CPU::call(bool condition = true) {
    if (condition) {
        u16 callAddress = bits::concatBytes(m_mmu.read(m_pc), m_mmu.read(m_pc+1));
        m_pc += 2;
        pushToStack(m_pc);
        m_pc = callAddress;
        cycleDelay += 6; // Opcode duration is extended on successful CALL
    } else {
        m_pc += 2;
    }
}

// Pops PC from stack
void CPU::ret(bool condition = true) {
    if (condition) {
        m_pc = popFromStack();
        cycleDelay += 5;  // Opcode duration is extended on successful RET
    }
}

// Restarts
void CPU::rst(u8 data) {
    pushToStack(m_pc);
    m_pc = data * 8;
}

// Interrupt instructions

// Enables interrupts
void CPU::ei() {
    m_interruptsEnabled = true;
}

// Disables interrupts
void CPU::di() {
    m_interruptsEnabled = false;
}

// Input/output instructions

// Gets data from IN port
void CPU::in(u8 portNum) {
    writeReg8(Reg8::A, m_ioPorts.in(portNum));
}

// Writes data to OUT port
void CPU::out(u8 portNum) {
    m_ioPorts.out(portNum, readReg8(Reg8::A));
}

// Halts CPU
void CPU::hlt() {
    m_isHalted = true;
}

void CPU::execute(u8 opcode) {
    using enum Reg8;
    using enum Reg16;
    switch (opcode) {
        case 0x00: break;
        case 0x01: lxi(BC, bits::concatBytes(m_mmu.read(m_pc), m_mmu.read(m_pc+1))); m_pc+=2; break;
        case 0x02: stax(BC); break;
        case 0x03: inx(BC); break;
        case 0x04: inr(B); break;
        case 0x05: dcr(B); break;
        case 0x06: mov(B, m_mmu.read(m_pc)); m_pc++; break;
        case 0x07: rlc(); break;
        case 0x08: break;
        case 0x09: dad(BC); break;
        case 0x0A: ldax(BC); break;
        case 0x0B: dcx(BC); break;
        case 0x0C: inr(C); break;
        case 0x0D: dcr(C); break;
        case 0x0E: mov(C, m_mmu.read(m_pc)); m_pc++; break;
        case 0x0F: rrc(); break;

        case 0x10: break;
        case 0x11: lxi(DE, bits::concatBytes(m_mmu.read(m_pc), m_mmu.read(m_pc+1))); m_pc+=2; break;
        case 0x12: stax(DE); break;
        case 0x13: inx(DE); break;
        case 0x14: inr(D); break;
        case 0x15: dcr(D); break;
        case 0x16: mov(D, m_mmu.read(m_pc)); m_pc++; break;
        case 0x17: ral(); break;
        case 0x18: break;
        case 0x19: dad(DE); break;
        case 0x1A: ldax(DE); break;
        case 0x1B: dcx(DE); break;
        case 0x1C: inr(E); break;
        case 0x1D: dcr(E); break;
        case 0x1E: mov(E, m_mmu.read(m_pc)); m_pc++; break;
        case 0x1F: rar(); break;

        case 0x20: break;
        case 0x21: lxi(HL, bits::concatBytes(m_mmu.read(m_pc), m_mmu.read(m_pc+1))); m_pc+=2; break;
        case 0x22: shld(bits::concatBytes(m_mmu.read(m_pc), m_mmu.read(m_pc+1))); m_pc+=2; break;
        case 0x23: inx(HL); break;
        case 0x24: inr(H); break;
        case 0x25: dcr(H); break;
        case 0x26: mov(H, m_mmu.read(m_pc)); m_pc++; break;
        case 0x27: daa(); break;
        case 0x28: break;
        case 0x29: dad(HL); break;
        case 0x2A: lhld(bits::concatBytes(m_mmu.read(m_pc), m_mmu.read(m_pc+1))); m_pc+=2; break;
        case 0x2B: dcx(HL); break;
        case 0x2C: inr(L); break;
        case 0x2D: dcr(L); break;
        case 0x2E: mov(L, m_mmu.read(m_pc)); m_pc++; break;
        case 0x2F: cma(); break;

        case 0x30: break;
        case 0x31: lxi(SP, bits::concatBytes(m_mmu.read(m_pc), m_mmu.read(m_pc+1))); m_pc+=2; break;
        case 0x32: sta(bits::concatBytes(m_mmu.read(m_pc), m_mmu.read(m_pc+1))); m_pc+=2; break;
        case 0x33: inx(SP); break;
        case 0x34: inr(M); break;
        case 0x35: dcr(M); break;
        case 0x36: mov(M, m_mmu.read(m_pc)); m_pc++; break;
        case 0x37: stc(); break;
        case 0x38: break;
        case 0x39: dad(SP); break;
        case 0x3A: lda(bits::concatBytes(m_mmu.read(m_pc), m_mmu.read(m_pc+1))); m_pc+=2; break;
        case 0x3B: dcx(SP); break;
        case 0x3C: inr(A); break;
        case 0x3D: dcr(A); break;
        case 0x3E: mov(A, m_mmu.read(m_pc)); m_pc++; break;
        case 0x3F: cmc(); break;

        case 0x40: mov(B, readReg8(B)); break;
        case 0x41: mov(B, readReg8(C)); break;
        case 0x42: mov(B, readReg8(D)); break;
        case 0x43: mov(B, readReg8(E)); break;
        case 0x44: mov(B, readReg8(H)); break;
        case 0x45: mov(B, readReg8(L)); break;
        case 0x46: mov(B, readReg8(M)); break;
        case 0x47: mov(B, readReg8(A)); break;
        case 0x48: mov(C, readReg8(B)); break;
        case 0x49: mov(C, readReg8(C)); break;
        case 0x4A: mov(C, readReg8(D)); break;
        case 0x4B: mov(C, readReg8(E)); break;
        case 0x4C: mov(C, readReg8(H)); break;
        case 0x4D: mov(C, readReg8(L)); break;
        case 0x4E: mov(C, readReg8(M)); break;
        case 0x4F: mov(C, readReg8(A)); break;

        case 0x50: mov(D, readReg8(B)); break;
        case 0x51: mov(D, readReg8(C)); break;
        case 0x52: mov(D, readReg8(D)); break;
        case 0x53: mov(D, readReg8(E)); break;
        case 0x54: mov(D, readReg8(H)); break;
        case 0x55: mov(D, readReg8(L)); break;
        case 0x56: mov(D, readReg8(M)); break;
        case 0x57: mov(D, readReg8(A)); break;
        case 0x58: mov(E, readReg8(B)); break;
        case 0x59: mov(E, readReg8(C)); break;
        case 0x5A: mov(E, readReg8(D)); break;
        case 0x5B: mov(E, readReg8(E)); break;
        case 0x5C: mov(E, readReg8(H)); break;
        case 0x5D: mov(E, readReg8(L)); break;
        case 0x5E: mov(E, readReg8(M)); break;
        case 0x5F: mov(E, readReg8(A)); break;

        case 0x60: mov(H, readReg8(B)); break;
        case 0x61: mov(H, readReg8(C)); break;
        case 0x62: mov(H, readReg8(D)); break;
        case 0x63: mov(H, readReg8(E)); break;
        case 0x64: mov(H, readReg8(H)); break;
        case 0x65: mov(H, readReg8(L)); break;
        case 0x66: mov(H, readReg8(M)); break;
        case 0x67: mov(H, readReg8(A)); break;
        case 0x68: mov(L, readReg8(B)); break;
        case 0x69: mov(L, readReg8(C)); break;
        case 0x6A: mov(L, readReg8(D)); break;
        case 0x6B: mov(L, readReg8(E)); break;
        case 0x6C: mov(L, readReg8(H)); break;
        case 0x6D: mov(L, readReg8(L)); break;
        case 0x6E: mov(L, readReg8(M)); break;
        case 0x6F: mov(L, readReg8(A)); break;

        case 0x70: mov(M, readReg8(B)); break;
        case 0x71: mov(M, readReg8(C)); break;
        case 0x72: mov(M, readReg8(D)); break;
        case 0x73: mov(M, readReg8(E)); break;
        case 0x74: mov(M, readReg8(H)); break;
        case 0x75: mov(M, readReg8(L)); break;
        case 0x76: hlt(); break;
        case 0x77: mov(M, readReg8(A)); break;
        case 0x78: mov(A, readReg8(B)); break;
        case 0x79: mov(A, readReg8(C)); break;
        case 0x7A: mov(A, readReg8(D)); break;
        case 0x7B: mov(A, readReg8(E)); break;
        case 0x7C: mov(A, readReg8(H)); break;
        case 0x7D: mov(A, readReg8(L)); break;
        case 0x7E: mov(A, readReg8(M)); break;
        case 0x7F: mov(A, readReg8(A)); break;

        case 0x80: add(readReg8(B)); break;
        case 0x81: add(readReg8(C)); break;
        case 0x82: add(readReg8(D)); break;
        case 0x83: add(readReg8(E)); break;
        case 0x84: add(readReg8(H)); break;
        case 0x85: add(readReg8(L)); break;
        case 0x86: add(readReg8(M)); break;
        case 0x87: add(readReg8(A)); break;
        case 0x88: adc(readReg8(B)); break;
        case 0x89: adc(readReg8(C)); break;
        case 0x8A: adc(readReg8(D)); break;
        case 0x8B: adc(readReg8(E)); break;
        case 0x8C: adc(readReg8(H)); break;
        case 0x8D: adc(readReg8(L)); break;
        case 0x8E: adc(readReg8(M)); break;
        case 0x8F: adc(readReg8(A)); break;

        case 0x90: sub(readReg8(B)); break;
        case 0x91: sub(readReg8(C)); break;
        case 0x92: sub(readReg8(D)); break;
        case 0x93: sub(readReg8(E)); break;
        case 0x94: sub(readReg8(H)); break;
        case 0x95: sub(readReg8(L)); break;
        case 0x96: sub(readReg8(M)); break;
        case 0x97: sub(readReg8(A)); break;
        case 0x98: sbb(readReg8(B)); break;
        case 0x99: sbb(readReg8(C)); break;
        case 0x9A: sbb(readReg8(D)); break;
        case 0x9B: sbb(readReg8(E)); break;
        case 0x9C: sbb(readReg8(H)); break;
        case 0x9D: sbb(readReg8(L)); break;
        case 0x9E: sbb(readReg8(M)); break;
        case 0x9F: sbb(readReg8(A)); break;

        case 0xA0: ana(readReg8(B)); break;
        case 0xA1: ana(readReg8(C)); break;
        case 0xA2: ana(readReg8(D)); break;
        case 0xA3: ana(readReg8(E)); break;
        case 0xA4: ana(readReg8(H)); break;
        case 0xA5: ana(readReg8(L)); break;
        case 0xA6: ana(readReg8(M)); break;
        case 0xA7: ana(readReg8(A)); break;
        case 0xA8: xra(readReg8(B)); break;
        case 0xA9: xra(readReg8(C)); break;
        case 0xAA: xra(readReg8(D)); break;
        case 0xAB: xra(readReg8(E)); break;
        case 0xAC: xra(readReg8(H)); break;
        case 0xAD: xra(readReg8(L)); break;
        case 0xAE: xra(readReg8(M)); break;
        case 0xAF: xra(readReg8(A)); break;

        case 0xB0: ora(readReg8(B)); break;
        case 0xB1: ora(readReg8(C)); break;
        case 0xB2: ora(readReg8(D)); break;
        case 0xB3: ora(readReg8(E)); break;
        case 0xB4: ora(readReg8(H)); break;
        case 0xB5: ora(readReg8(L)); break;
        case 0xB6: ora(readReg8(M)); break;
        case 0xB7: ora(readReg8(A)); break;
        case 0xB8: cmp(readReg8(B)); break;
        case 0xB9: cmp(readReg8(C)); break;
        case 0xBA: cmp(readReg8(D)); break;
        case 0xBB: cmp(readReg8(E)); break;
        case 0xBC: cmp(readReg8(H)); break;
        case 0xBD: cmp(readReg8(L)); break;
        case 0xBE: cmp(readReg8(M)); break;
        case 0xBF: cmp(readReg8(A)); break;

        case 0xC0: ret(getZero() == 0); break;
        case 0xC1: pop(BC); break;
        case 0xC2: jmp(getZero() == 0); break;
        case 0xC3: jmp(); break;
        case 0xC4: call(getZero() == 0); break;
        case 0xC5: push(BC); break;
        case 0xC6: add(m_mmu.read(m_pc)); m_pc++; break;
        case 0xC7: rst(0); break;
        case 0xC8: ret(getZero() == 1); break;
        case 0xC9: ret(); break;
        case 0xCA: jmp(getZero() == 1); break;
        case 0xCB: jmp(); break;
        case 0xCC: call(getZero() == 1); break;
        case 0xCD: call(); break;
        case 0xCE: adc(m_mmu.read(m_pc)); m_pc++; break;
        case 0xCF: rst(1); break;

        case 0xD0: ret(getCarry() == 0); break;
        case 0xD1: pop(DE); break;
        case 0xD2: jmp(getCarry() == 0); break;
        case 0xD3: out(m_mmu.read(m_pc)); m_pc++; break;
        case 0xD4: call(getCarry() == 0); break;
        case 0xD5: push(DE); break;
        case 0xD6: sub(m_mmu.read(m_pc)); m_pc++; break;
        case 0xD7: rst(2); break;
        case 0xD8: ret(getCarry() == 1); break;
        case 0xD9: ret(); break;
        case 0xDA: jmp(getCarry() == 1); break;
        case 0xDB: in(m_mmu.read(m_pc)); m_pc++; break;
        case 0xDC: call(getCarry() == 1); break;
        case 0xDD: call(); break;
        case 0xDE: sbb(m_mmu.read(m_pc)); m_pc++; break;
        case 0xDF: rst(3); break;

        case 0xE0: ret(getParity() == 0); break;
        case 0xE1: pop(HL); break;
        case 0xE2: jmp(getParity() == 0); break;
        case 0xE3: xthl(); break;
        case 0xE4: call(getParity() == 0); break;
        case 0xE5: push(HL); break;
        case 0xE6: ana(m_mmu.read(m_pc)); m_pc++; break;
        case 0xE7: rst(4); break;
        case 0xE8: ret(getParity() == 1); break;
        case 0xE9: pchl(); break;
        case 0xEA: jmp(getParity() == 1); break;
        case 0xEB: xchg(); break;
        case 0xEC: call(getParity() == 1); break;
        case 0xED: call(); break;
        case 0xEE: xra(m_mmu.read(m_pc)); m_pc++; break;
        case 0xEF: rst(5); break;

        case 0xF0: ret(getSign() == 0); break;
        case 0xF1: pop(AF); break;
        case 0xF2: jmp(getSign() == 0); break;
        case 0xF3: di(); break;
        case 0xF4: call(getSign() == 0); break;
        case 0xF5: push(AF); break;
        case 0xF6: ora(m_mmu.read(m_pc)); m_pc++; break;
        case 0xF7: rst(6); break;
        case 0xF8: ret(getSign() == 1); break;
        case 0xF9: sphl(); break;
        case 0xFA: jmp(getSign() == 1); break;
        case 0xFB: ei(); break;
        case 0xFC: call(getSign() == 1); break;
        case 0xFD: call(); break;
        case 0xFE: cmp(m_mmu.read(m_pc)); m_pc++; break;
        case 0xFF: rst(7); break;
    }
}
