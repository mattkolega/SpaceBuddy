#include "cpu.h"

#include "bus.h"

#include <common/bitwise.h>
#include <common/types.h>

u16 RegisterPair::get() const { return Bitwise::concatBytes(lo, hi); }
void RegisterPair::set(u16 newVal) { lo = newVal & 0xFF; hi = newVal >> 8; }

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

CPU::CPU(Bus& bus) : bus(bus) {}

// Returns a reference to register m which is an alias for the memory value at address HL
u8& CPU::m() {
    return bus.getMemRef(hl.get());
}

void CPU::step() {
    if (cycleDelay == 0) {
        if (isHalted) return;

        u8 opcode {};

        if (midFrameInterrupt && interruptsEnabled) {
            opcode = 0xCF;
            midFrameInterrupt = false;
            interruptsEnabled = false;
        } else if (endFrameInterrupt && interruptsEnabled) {
            opcode = 0xD7;
            endFrameInterrupt = false;
            interruptsEnabled = false;
        } else {
            opcode = bus.memRead(pc);
            pc++;
        }

        execute(opcode);
        auto opcodeInfo = opcodeInfoTable[opcode];
        cycleDelay = opcodeInfo.cycles;
    };
    cycleDelay--;
}

void CPU::triggerInterrupt(InterruptType interruptType) {
    using enum InterruptType;

    if (interruptType == MidFrame) {
        midFrameInterrupt = true;
    } else if (interruptType == EndFrame) {
        endFrameInterrupt = true;
    }
}

void CPU::pushToStack(u16 value) {
    sp--;
    bus.memWrite(sp, (value >> 8) & 0xFF);
    sp--;
    bus.memWrite(sp, value & 0xFF);
}

u16 CPU::popFromStack() {
    u8 lo = bus.memRead(sp);
    sp++;
    u8 hi = bus.memRead(sp);
    sp++;
    return Bitwise::concatBytes(lo, hi);
}

u8 CPU::getCarry() const { return Bitwise::getBitInByte(f, static_cast<u8>(FlagBits::C)); }
void CPU::setCarry(bool value) { f = Bitwise::modifyBitInByte(f, static_cast<u8>(FlagBits::C), value); }

u8 CPU::getParity() const { return Bitwise::getBitInByte(f, static_cast<u8>(FlagBits::P)); }
void CPU::setParity(bool value) { f = Bitwise::modifyBitInByte(f, static_cast<u8>(FlagBits::P), value); }

u8 CPU::getAuxCarry() const { return Bitwise::getBitInByte(f, static_cast<u8>(FlagBits::A)); }
void CPU::setAuxCarry(bool value) { f = Bitwise::modifyBitInByte(f, static_cast<u8>(FlagBits::A), value); }

u8 CPU::getZero() const { return Bitwise::getBitInByte(f, static_cast<u8>(FlagBits::Z)); }
void CPU::setZero(bool value) { f = Bitwise::modifyBitInByte(f, static_cast<u8>(FlagBits::Z), value); }

u8 CPU::getSign() const { return Bitwise::getBitInByte(f, static_cast<u8>(FlagBits::S)); }
void CPU::setSign(bool value) { f = Bitwise::modifyBitInByte(f, static_cast<u8>(FlagBits::S), value); }

/* INSTRUCTIONS */

// Carry bit instructions

// Complements carry flag
void CPU::cmc() {
    setCarry(~getCarry());
}

// Sets carry flag
void CPU::stc() {
    setCarry(1);
}

// Single register instructions

// Increments register
void CPU::inr(u8& reg) {
    u8 oldValue = reg;
    reg++;

    setZero(reg == 0);
    setSign(Bitwise::isNegative(reg));
    setParity(Bitwise::checkParity(reg));
    setAuxCarry(Bitwise::checkHalfCarryAdd(oldValue, 1));
}

// Decrements register
void CPU::dcr(u8& reg) {
    u8 oldValue = reg;
    reg--;

    setZero(reg == 0);
    setSign(Bitwise::isNegative(reg));
    setParity(Bitwise::checkParity(reg));
    setAuxCarry(Bitwise::checkHalfCarrySub(oldValue, 1));
}

// Complements accumulator
void CPU::cma() {
    a = ~a;
}

// Decimal adjusts accumulator
void CPU::daa() {
    u8 offset { 0 };

    if (getSign() == 0 && (a & 0xF) > 0x9 || getAuxCarry() == 1) {
        offset |= 0x06;
        setAuxCarry(1);
    } else {
        setAuxCarry(0);
    }

    if (getSign() == 0 && a > 0x99 || getCarry() == 1) {
        offset |= 0x60;
        setCarry(1);
    } else {
        setCarry(0);
    }

    (getSign() == 0) ? a += offset : a -= offset;

    // Set flags
    setZero(a == 0);
    setSign(Bitwise::isNegative(a));
    setParity(Bitwise::checkParity(a));
}

// Data transfer instructions

// Moves data to register
void CPU::mov(u8& dst, u8 src) {
    dst = src;
}

// Stores accumulator at memory address provided by BC or DE
void CPU::stax(u16 reg) {
    bus.memWrite(reg, a);
}

// Loads accumulator with memory value at address provided by BC or DE
void CPU::ldax(u16 reg) {
    a = bus.memRead(reg);
}

// Arithmetic instructions

// Adds register value to accumulator
void CPU::add(u8 reg) {
    u8 originalValue = a;
    a += reg;

    setZero(a == 0);
    setSign(Bitwise::isNegative(a));
    setParity(Bitwise::checkParity(a));
    setAuxCarry(Bitwise::checkHalfCarryAdd(originalValue, reg));
    setCarry((originalValue + reg) > 0xFF);
};

// Adds register value + carry to accumulator
void CPU::adc(u8 reg) {
    u8 originalValue = a;
    a += reg + getCarry();

    setZero(a == 0);
    setSign(Bitwise::isNegative(a));
    setParity(Bitwise::checkParity(a));
    setAuxCarry(Bitwise::checkHalfCarryAdd(originalValue, reg, getCarry()));
    setCarry((originalValue + reg + getCarry()) > 0xFF);
};

// Subtracts register value from accumulator
void CPU::sub(u8 reg) {
    u8 originalValue = a;
    a -= reg;

    setZero(a == 0);
    setSign(Bitwise::isNegative(a));
    setParity(Bitwise::checkParity(a));
    setAuxCarry(Bitwise::checkHalfCarrySub(originalValue, reg));
    setCarry((originalValue - reg) < 0);
};

// Subtracts register value + carry from accumulator
void CPU::sbb(u8 reg) {
    u8 originalValue = a;
    a -= reg - getCarry();

    setZero(a == 0);
    setSign(Bitwise::isNegative(a));
    setParity(Bitwise::checkParity(a));
    setAuxCarry(Bitwise::checkHalfCarrySub(originalValue, reg, getCarry()));
    setCarry((originalValue - reg - getCarry()) < 0);
};

// Logical instructions

// ANDs register with accumulator
void CPU::ana(u8 reg) {
    a &= reg;

    setZero(a == 0);
    setSign(Bitwise::isNegative(a));
    setParity(Bitwise::checkParity(a));
    setCarry(0);
}

// XORs register with accumulator
void CPU::xra(u8 reg) {
    a ^= reg;

    setZero(a == 0);
    setSign(Bitwise::isNegative(a));
    setParity(Bitwise::checkParity(a));
    setAuxCarry(0);
    setCarry(0);
}

// ORs register with accumulator
void CPU::ora(u8 reg) {
    a |= reg;

    setZero(a == 0);
    setSign(Bitwise::isNegative(a));
    setParity(Bitwise::checkParity(a));
    setAuxCarry(0);
    setCarry(0);
}

// Compares register to accumulator
void CPU::cmp(u8 reg) {
    u8 result = a - reg;

    setZero(a == reg);
    setSign(Bitwise::isNegative(result));
    setParity(Bitwise::checkParity(result));
    setAuxCarry(Bitwise::checkHalfCarrySub(a, reg));
    setCarry(a < reg);
}

// Rotation instructions

// Rotate accumulator left
void CPU::rlc() {
    setCarry(Bitwise::getBitInByte(a, 7)); // Set carry to leftmost bit
    a = std::rotl(a, 1);
}

// Rotate accumulator right
void CPU::rrc() {
    setCarry(Bitwise::getBitInByte(a, 0)); // Set carry to rightmost bit
    a = std::rotr(a, 1);
}

// Rotate accumulator left through carry
void CPU::ral() {
    u8 carry = getCarry();
    setCarry(Bitwise::getBitInByte(a, 7)); // Set carry flag to leftmost bit
    u8 rotated = std::rotl(a, 1);
    a = Bitwise::modifyBitInByte(rotated, 0, carry);
}

// Rotate accumulator right through carry
void CPU::rar() {
    u8 carry = getCarry();
    setCarry(Bitwise::getBitInByte(a, 0));  // Set carry flag to rightmost bit
    u8 rotated = std::rotr(a, 1);
    a = Bitwise::modifyBitInByte(rotated, 7, carry);
}

// Register pair instructions

// Pushes register pair to stack
void CPU::push(RegisterPair reg) {
    pushToStack(reg.get());
}

// Pops register pair from stack
void CPU::pop(RegisterPair &reg) {
    reg.set(popFromStack());
}

// Adds register pair to HL
void CPU::dad(u16 reg){
    u16 originalValue = hl.get();
    hl.set(hl.get() + reg);
    setCarry((originalValue + reg) > 0xFFFF);
}

// Increments register pair
void CPU::inx(RegisterPair& reg) {
    reg.set(reg.get() + 1);
}

// Increments word
void CPU::inx(u16& reg) {
    reg++;
}

// Decrements register pair
void CPU::dcx(RegisterPair& reg){
    reg.set(reg.get() - 1);
}

// Decrements word
void CPU::dcx(u16& reg){
    reg--;
}

// Swaps DE and HL values
void CPU::xchg(){
    std::swap(d, h);
    std::swap(e, l);
}

// Swaps HL and top word on stack
void CPU::xthl(){
    u8 lo = bus.memRead(sp);
    bus.memWrite(sp, l);
    u8 hi = bus.memRead(sp+1);
    bus.memWrite(sp+1, h);
    hl.set(Bitwise::concatBytes(lo, hi));
}

// Sets SP to HL
void CPU::sphl(){
    sp = hl.get();
}

// Immediate instructions

// Loads immediate value into register pair
void CPU::lxi(RegisterPair& reg, u16 immediate) {
    reg.set(immediate);
}

// Loads immediate value into word-sized register
void CPU::lxi(u16& reg, u16 immediate) {
    reg = immediate;
}

// Direct addressing instructions

// Stores accumulator in memory
void CPU::sta(u16 address) {
    bus.memWrite(address, a);
}

// Loads accumulator from memory
void CPU::lda(u16 address) {
    a = bus.memRead(address);
}

// Stores HL in memory
void CPU::shld(u16 address) {
    bus.memWrite(address, hl.lo);
    bus.memWrite(address+1, hl.hi);
}

// Loads HL from memory
void CPU::lhld(u16 address) {
    hl.lo = bus.memRead(address);
    hl.hi = bus.memRead(address+1);
}

// Jump instructions

// Jumps to address in HL
void CPU::pchl() {
    pc = hl.get();
}

// Jumps to address
void CPU::jmp(bool condition = true) {
    if (condition) pc = Bitwise::concatBytes(bus.memRead(pc), bus.memRead(pc+1));
}

// Call subroutine instructions

// Pushes PC to stack and jumps to address
void CPU::call(bool condition = true) {
    if (condition) {
        pushToStack(pc);
        pc = Bitwise::concatBytes(bus.memRead(pc), bus.memRead(pc+1));
        cycleDelay += 6; // Opcode duration is extended on successful CALL
    }
}

// Pops PC from stack
void CPU::ret(bool condition = true) {
    if (condition) {
        pc = popFromStack();
        cycleDelay += 5;  // Opcode duration is extended on successful RET
    }
}

// Restarts
void CPU::rst(u8 data) {
    pushToStack(pc);
    pc = data * 8;
}

// Interrupt instructions

// Enables interrupts
void CPU::ei() {
    interruptsEnabled = true;
}

// Disables interrupts
void CPU::di() {
    interruptsEnabled = false;
}

// Input/output instructions

// Gets data from IN port
void CPU::in(u8 portNum) {
    a = bus.in(portNum);
}

// Writes data to OUT port
void CPU::out(u8 portNum) {
    bus.out(portNum, a);
}

// Halts CPU
void CPU::hlt() {
    isHalted = true;
}

void CPU::execute(u8 opcode) {
    switch (opcode) {
        case 0x00: break;
        case 0x01: lxi(bc, Bitwise::concatBytes(bus.memRead(pc), bus.memRead(pc+1))); pc+=2; break;
        case 0x02: stax(bc.get()); break;
        case 0x03: inx(bc); break;
        case 0x04: inr(b); break;
        case 0x05: dcr(b); break;
        case 0x06: mov(b, bus.memRead(pc)); pc++; break;
        case 0x07: rlc(); break;
        case 0x08: break;
        case 0x09: dad(bc.get()); break;
        case 0x0A: ldax(bc.get()); break;
        case 0x0B: dcx(bc); break;
        case 0x0C: inr(c); break;
        case 0x0D: dcr(c); break;
        case 0x0E: mov(c, bus.memRead(pc)); pc++; break;
        case 0x0F: rrc(); break;

        case 0x10: break;
        case 0x11: lxi(de, Bitwise::concatBytes(bus.memRead(pc), bus.memRead(pc+1))); pc+=2; break;
        case 0x12: stax(de.get()); break;
        case 0x13: inx(de); break;
        case 0x14: inr(d); break;
        case 0x15: dcr(d); break;
        case 0x16: mov(d, bus.memRead(pc)); pc++; break;
        case 0x17: ral(); break;
        case 0x18: break;
        case 0x19: dad(de.get()); break;
        case 0x1A: ldax(de.get()); break;
        case 0x1B: dcx(de); break;
        case 0x1C: inr(e); break;
        case 0x1D: dcr(e); break;
        case 0x1E: mov(e, bus.memRead(pc)); pc++; break;
        case 0x1F: rar(); break;

        case 0x20: break;
        case 0x21: lxi(hl, Bitwise::concatBytes(bus.memRead(pc), bus.memRead(pc+1))); pc+=2; break;
        case 0x22: shld(Bitwise::concatBytes(bus.memRead(pc), bus.memRead(pc+1))); pc+=2; break;
        case 0x23: inx(hl); break;
        case 0x24: inr(h); break;
        case 0x25: dcr(h); break;
        case 0x26: mov(h, bus.memRead(pc)); pc++; break;
        case 0x27: daa(); break;
        case 0x28: break;
        case 0x29: dad(hl.get()); break;
        case 0x2A: lhld(Bitwise::concatBytes(bus.memRead(pc), bus.memRead(pc+1))); pc+=2; break;
        case 0x2B: dcx(hl); break;
        case 0x2C: inr(l); break;
        case 0x2D: dcr(l); break;
        case 0x2E: mov(l, bus.memRead(pc)); pc++; break;
        case 0x2F: cma(); break;

        case 0x30: break;
        case 0x31: lxi(sp, Bitwise::concatBytes(bus.memRead(pc), bus.memRead(pc+1))); pc+=2; break;
        case 0x32: sta(Bitwise::concatBytes(bus.memRead(pc), bus.memRead(pc+1))); pc+=2; break;
        case 0x33: inx(sp); break;
        case 0x34: inr(m()); break;
        case 0x35: dcr(m()); break;
        case 0x36: mov(m(), bus.memRead(pc)); pc++; break;
        case 0x37: stc(); break;
        case 0x38: break;
        case 0x39: dad(sp); break;
        case 0x3A: lda(Bitwise::concatBytes(bus.memRead(pc), bus.memRead(pc+1))); pc+=2; break;
        case 0x3B: dcx(sp); break;
        case 0x3C: inr(a); break;
        case 0x3D: dcr(a); break;
        case 0x3E: mov(a, bus.memRead(pc)); pc++; break;
        case 0x3F: cmc(); break;

        case 0x40: mov(b, b); break;
        case 0x41: mov(b, c); break;
        case 0x42: mov(b, d); break;
        case 0x43: mov(b, e); break;
        case 0x44: mov(b, h); break;
        case 0x45: mov(b, l); break;
        case 0x46: mov(b, m()); break;
        case 0x47: mov(b, a); break;
        case 0x48: mov(c, b); break;
        case 0x49: mov(c, c); break;
        case 0x4A: mov(c, d); break;
        case 0x4B: mov(c, e); break;
        case 0x4C: mov(c, h); break;
        case 0x4D: mov(c, l); break;
        case 0x4E: mov(c, m()); break;
        case 0x4F: mov(c, a); break;

        case 0x50: mov(d, b); break;
        case 0x51: mov(d, c); break;
        case 0x52: mov(d, d); break;
        case 0x53: mov(d, e); break;
        case 0x54: mov(d, h); break;
        case 0x55: mov(d, l); break;
        case 0x56: mov(d, m()); break;
        case 0x57: mov(d, a); break;
        case 0x58: mov(e, b); break;
        case 0x59: mov(e, c); break;
        case 0x5A: mov(e, d); break;
        case 0x5B: mov(e, e); break;
        case 0x5C: mov(e, h); break;
        case 0x5D: mov(e, l); break;
        case 0x5E: mov(e, m()); break;
        case 0x5F: mov(e, a); break;

        case 0x60: mov(h, b); break;
        case 0x61: mov(h, c); break;
        case 0x62: mov(h, d); break;
        case 0x63: mov(h, e); break;
        case 0x64: mov(h, h); break;
        case 0x65: mov(h, l); break;
        case 0x66: mov(h, m()); break;
        case 0x67: mov(h, a); break;
        case 0x68: mov(l, b); break;
        case 0x69: mov(l, c); break;
        case 0x6A: mov(l, d); break;
        case 0x6B: mov(l, e); break;
        case 0x6C: mov(l, h); break;
        case 0x6D: mov(l, l); break;
        case 0x6E: mov(l, m()); break;
        case 0x6F: mov(l, a); break;

        case 0x70: mov(m(), b); break;
        case 0x71: mov(m(), c); break;
        case 0x72: mov(m(), d); break;
        case 0x73: mov(m(), e); break;
        case 0x74: mov(m(), h); break;
        case 0x75: mov(m(), l); break;
        case 0x76: hlt(); break;
        case 0x77: mov(m(), a); break;
        case 0x78: mov(a, b); break;
        case 0x79: mov(a, c); break;
        case 0x7A: mov(a, d); break;
        case 0x7B: mov(a, e); break;
        case 0x7C: mov(a, h); break;
        case 0x7D: mov(a, l); break;
        case 0x7E: mov(a, m()); break;
        case 0x7F: mov(a, a); break;

        case 0x80: add(b); break;
        case 0x81: add(c); break;
        case 0x82: add(d); break;
        case 0x83: add(e); break;
        case 0x84: add(h); break;
        case 0x85: add(l); break;
        case 0x86: add(m()); break;
        case 0x87: add(a); break;
        case 0x88: adc(b); break;
        case 0x89: adc(c); break;
        case 0x8A: adc(d); break;
        case 0x8B: adc(e); break;
        case 0x8C: adc(h); break;
        case 0x8D: adc(l); break;
        case 0x8E: adc(m()); break;
        case 0x8F: adc(a); break;

        case 0x90: sub(b); break;
        case 0x91: sub(c); break;
        case 0x92: sub(d); break;
        case 0x93: sub(e); break;
        case 0x94: sub(h); break;
        case 0x95: sub(l); break;
        case 0x96: sub(m()); break;
        case 0x97: sub(a); break;
        case 0x98: sbb(b); break;
        case 0x99: sbb(c); break;
        case 0x9A: sbb(d); break;
        case 0x9B: sbb(e); break;
        case 0x9C: sbb(h); break;
        case 0x9D: sbb(l); break;
        case 0x9E: sbb(m()); break;
        case 0x9F: sbb(a); break;

        case 0xA0: ana(b); break;
        case 0xA1: ana(c); break;
        case 0xA2: ana(d); break;
        case 0xA3: ana(e); break;
        case 0xA4: ana(h); break;
        case 0xA5: ana(l); break;
        case 0xA6: ana(m()); break;
        case 0xA7: ana(a); break;
        case 0xA8: xra(b); break;
        case 0xA9: xra(c); break;
        case 0xAA: xra(d); break;
        case 0xAB: xra(e); break;
        case 0xAC: xra(h); break;
        case 0xAD: xra(l); break;
        case 0xAE: xra(m()); break;
        case 0xAF: xra(a); break;

        case 0xB0: ora(b); break;
        case 0xB1: ora(c); break;
        case 0xB2: ora(d); break;
        case 0xB3: ora(e); break;
        case 0xB4: ora(h); break;
        case 0xB5: ora(l); break;
        case 0xB6: ora(m()); break;
        case 0xB7: ora(a); break;
        case 0xB8: cmp(b); break;
        case 0xB9: cmp(c); break;
        case 0xBA: cmp(d); break;
        case 0xBB: cmp(e); break;
        case 0xBC: cmp(h); break;
        case 0xBD: cmp(l); break;
        case 0xBE: cmp(m()); break;
        case 0xBF: cmp(a); break;

        case 0xC0: ret(getZero() == 0); break;
        case 0xC1: pop(bc); break;
        case 0xC2: jmp(getZero() == 0); break;
        case 0xC3: jmp(); break;
        case 0xC4: call(getZero() == 0); break;
        case 0xC5: push(bc); break;
        case 0xC6: add(bus.memRead(pc)); pc++; break;
        case 0xC7: rst(0); break;
        case 0xC8: ret(); break;
        case 0xC9: ret(getZero() == 1); break;
        case 0xCA: jmp(getZero() == 1); break;
        case 0xCB: jmp(); break;
        case 0xCC: call(getZero() == 1); break;
        case 0xCD: call(); break;
        case 0xCE: adc(bus.memRead(pc)); pc++; break;
        case 0xCF: rst(1); break;

        case 0xD0: ret(getCarry() == 0); break;
        case 0xD1: pop(de); break;
        case 0xD2: jmp(getCarry() == 0); break;
        case 0xD3: out(bus.memRead(pc)); pc++; break;
        case 0xD4: call(getCarry() == 0); break;
        case 0xD5: push(de); break;
        case 0xD6: sub(bus.memRead(pc)); pc++; break;
        case 0xD7: rst(2); break;
        case 0xD8: ret(); break;
        case 0xD9: ret(getCarry() == 1); break;
        case 0xDA: jmp(getCarry() == 1); break;
        case 0xDB: in(bus.memRead(pc)); pc++; break;
        case 0xDC: call(getCarry() == 1); break;
        case 0xDD: call(); break;
        case 0xDE: sbb(bus.memRead(pc)); pc++; break;
        case 0xDF: rst(3); break;

        case 0xE0: ret(getParity() == 0); break;
        case 0xE1: pop(hl); break;
        case 0xE2: jmp(getParity() == 0); break;
        case 0xE3: xthl(); break;
        case 0xE4: call(getParity() == 0); break;
        case 0xE5: push(hl); break;
        case 0xE6: ana(bus.memRead(pc)); pc++; break;
        case 0xE7: rst(4); break;
        case 0xE8: pchl(); break;
        case 0xE9: ret(getParity() == 1); break;
        case 0xEA: jmp(getParity() == 1); break;
        case 0xEB: xchg(); break;
        case 0xEC: call(getParity() == 1); break;
        case 0xED: call(); break;
        case 0xEE: xra(bus.memRead(pc)); pc++; break;
        case 0xEF: rst(5); break;

        case 0xF0: ret(getSign() == 0); break;
        case 0xF1: pop(psw); break;
        case 0xF2: jmp(getSign() == 0); break;
        case 0xF3: di(); break;
        case 0xF4: call(getSign() == 0); break;
        case 0xF5: push(psw); break;
        case 0xF6: ora(bus.memRead(pc)); pc++; break;
        case 0xF7: rst(6); break;
        case 0xF8: sphl(); break;
        case 0xF9: ret(getSign() == 1); break;
        case 0xFA: jmp(getSign() == 1); break;
        case 0xFB: ei(); break;
        case 0xFC: call(getSign() == 1); break;
        case 0xFD: call(); break;
        case 0xFE: cmp(bus.memRead(pc)); pc++; break;
        case 0xFF: rst(7); break;
    }
}