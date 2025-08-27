#include <utility>

#include "cpu.h"

#include <common/bitwise.h>

u16 RegisterPair::get() const { return Bitwise::concatBytes(lo, hi); }
void RegisterPair::set(u16 newVal) { lo = newVal & 0xFF; hi = newVal >> 8; }

void CPU::pushToStack(u16 value) {
    sp--;
    mmu.memWrite(sp, (value >> 8) & 0xFF);
    sp--;
    mmu.memWrite(sp, value & 0xFF);
}

u16 CPU::popFromStack() {
    u8 lo = mmu.memRead(sp);
    sp++;
    u8 hi = mmu.memRead(sp);
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
    mmu.memWrite(reg, a);
}

// Loads accumulator with memory value at address provided by BC or DE
void CPU::ldax(u16 reg) {
    a = mmu.memRead(reg);
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
    reg = popFromStack();
}

// Adds register pair to HL
void CPU::dad(u16 reg){
    u8 originalValue = hl.get();
    hl.set(hl.get() + reg);
    setCarry((originalValue + reg) > 0xFFFF);
}

// Increments register pair
void inx(RegisterPair& reg) {
    reg.set(reg.get() + 1);
}

// Increments word
void inx(u16& reg) {
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
    u8 lo = mmu.memRead(sp);
    mmu.memWrite(sp, l);
    u8 hi = mmu.memRead(sp+1);
    mmu.memWrite(sp+1, h);
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
    mmu.memWrite(address, a);
}

// Loads accumulator from memory
void CPU::lda(u16 address) {
    a = mmu.memRead(address);
}

// Stores HL in memory
void CPU::shld(u16 address) {
    mmu.memWrite(address, hl.get());
}

// Loads HL from memory
void CPU::lhld(u16 address) {
    hl.set(mmu.memRead(address));
}

// Jump instructions

// Jumps to address in HL
void CPU::pchl() {
    pc = hl.get();
}

// Jumps to address
void CPU::jmp(bool condition = true) {
    if (condition) pc = Bitwise::concatBytes(mmu.memRead(pc), mmu.memRead(pc+1));
}

// Call subroutine instructions

// Pushes PC to stack and jumps to address
void CPU::call(bool condition = true) {
    if (condition) {
        pushToStack(pc);
        pc = Bitwise::concatBytes(mmu.memRead(pc), mmu.memRead(pc+1));
    }
}

// Pops PC from stack
void CPU::ret(bool condition = true) {
    if (condition) pc = popFromStack();
}

// Restarts
void CPU::rst(u8 data) {
    pushToStack(pc);
    pc = data * 8;
}

void CPU::execute() {
    u8 opcode = mmu.memRead(pc);
    pc++;

    switch (opcode) {
        case 0x00: break;
        case 0x01: lxi(bc, Bitwise::concatBytes(mmu.memRead(pc), mmu.memRead(pc+1))); pc+=2; break;
        case 0x02: stax(bc.get()); break;
        case 0x03: inx(bc); break;
        case 0x04: inr(b); break;
        case 0x05: dcr(b); break;
        case 0x06: mov(b, mmu.memRead(pc)); pc++; break;
        case 0x07: rlc(); break;
        case 0x08: break;
        case 0x09: dad(bc.get()); break;
        case 0x0A: ldax(bc.get()); break;
        case 0x0B: dcx(bc); break;
        case 0x0C: inr(c); break;
        case 0x0D: dcr(c); break;
        case 0x0E: mov(c, mmu.memRead(pc)); pc++; break;
        case 0x0F: rrc(); break;

        case 0x10: break;
        case 0x11: lxi(de, Bitwise::concatBytes(mmu.memRead(pc), mmu.memRead(pc+1))); pc+=2; break;
        case 0x12: stax(de.get()); break;
        case 0x13: inx(de); break;
        case 0x14: inr(d); break;
        case 0x15: dcr(d); break;
        case 0x16: mov(d, mmu.memRead(pc)); pc++; break;
        case 0x17: ral(); break;
        case 0x18: break;
        case 0x19: dad(de.get()); break;
        case 0x1A: ldax(de.get()); break;
        case 0x1B: dcx(de); break;
        case 0x1C: inr(e); break;
        case 0x1D: dcr(e); break;
        case 0x1E: mov(e, mmu.memRead(pc)); pc++; break;
        case 0x1F: rar(); break;

        case 0x20: break;
        case 0x21: lxi(hl, Bitwise::concatBytes(mmu.memRead(pc), mmu.memRead(pc+1))); pc+=2; break;
        case 0x22: shld(pc); pc++; break;
        case 0x23: inx(hl); break;
        case 0x24: inr(h); break;
        case 0x25: dcr(h); break;
        case 0x26: mov(h, mmu.memRead(pc)); pc++; break;
        case 0x27: daa(); break;
        case 0x28: break;
        case 0x29: dad(hl.get()); break;
        case 0x2A: lhld(pc); pc++; break;
        case 0x2B: dcx(hl); break;
        case 0x2C: inr(l); break;
        case 0x2D: dcr(l); break;
        case 0x2E: mov(l, mmu.memRead(pc)); pc++; break;
        case 0x2F: cma(); break;

        case 0x30: break;
        case 0x31: lxi(sp, Bitwise::concatBytes(mmu.memRead(pc), mmu.memRead(pc+1))); pc+=2; break;
        case 0x32: sta(pc); pc++; break;
        case 0x33: inx(sp); break;
        case 0x34: inr(m); break;
        case 0x35: dcr(m); break;
        case 0x36: mov(m, mmu.memRead(pc)); pc++; break;
        case 0x37: stc(); break;
        case 0x38: break;
        case 0x39: dad(sp); break;
        case 0x3A: lda(pc); pc++; break;
        case 0x3B: dcx(sp); break;
        case 0x3C: inr(a); break;
        case 0x3D: dcr(a); break;
        case 0x3E: mov(a, mmu.memRead(pc)); pc++; break;
        case 0x3F: cmc(); break;

        case 0x40: mov(b, b); break;
        case 0x41: mov(b, c); break;
        case 0x42: mov(b, d); break;
        case 0x43: mov(b, e); break;
        case 0x44: mov(b, h); break;
        case 0x45: mov(b, l); break;
        case 0x46: mov(b, m); break;
        case 0x47: mov(b, a); break;
        case 0x48: mov(c, b); break;
        case 0x49: mov(c, c); break;
        case 0x4A: mov(c, d); break;
        case 0x4B: mov(c, e); break;
        case 0x4C: mov(c, h); break;
        case 0x4D: mov(c, l); break;
        case 0x4E: mov(c, m); break;
        case 0x4F: mov(c, a); break;

        case 0x50: mov(d, b); break;
        case 0x51: mov(d, c); break;
        case 0x52: mov(d, d); break;
        case 0x53: mov(d, e); break;
        case 0x54: mov(d, h); break;
        case 0x55: mov(d, l); break;
        case 0x56: mov(d, m); break;
        case 0x57: mov(d, a); break;
        case 0x58: mov(e, b); break;
        case 0x59: mov(e, c); break;
        case 0x5A: mov(e, d); break;
        case 0x5B: mov(e, e); break;
        case 0x5C: mov(e, h); break;
        case 0x5D: mov(e, l); break;
        case 0x5E: mov(e, m); break;
        case 0x5F: mov(e, a); break;

        case 0x60: mov(h, b); break;
        case 0x61: mov(h, c); break;
        case 0x62: mov(h, d); break;
        case 0x63: mov(h, e); break;
        case 0x64: mov(h, h); break;
        case 0x65: mov(h, l); break;
        case 0x66: mov(h, m); break;
        case 0x67: mov(h, a); break;
        case 0x68: mov(l, b); break;
        case 0x69: mov(l, c); break;
        case 0x6A: mov(l, d); break;
        case 0x6B: mov(l, e); break;
        case 0x6C: mov(l, h); break;
        case 0x6D: mov(l, l); break;
        case 0x6E: mov(l, m); break;
        case 0x6F: mov(l, a); break;

        case 0x70: mov(m, b); break;
        case 0x71: mov(m, c); break;
        case 0x72: mov(m, d); break;
        case 0x73: mov(m, e); break;
        case 0x74: mov(m, h); break;
        case 0x75: mov(m, l); break;
        case 0x76: hlt(); break;
        case 0x77: mov(m, a); break;
        case 0x78: mov(a, b); break;
        case 0x79: mov(a, c); break;
        case 0x7A: mov(a, d); break;
        case 0x7B: mov(a, e); break;
        case 0x7C: mov(a, h); break;
        case 0x7D: mov(a, l); break;
        case 0x7E: mov(a, m); break;
        case 0x7F: mov(a, a); break;

        case 0x80: add(b); break;
        case 0x81: add(c); break;
        case 0x82: add(d); break;
        case 0x83: add(e); break;
        case 0x84: add(h); break;
        case 0x85: add(l); break;
        case 0x86: add(m); break;
        case 0x87: add(a); break;
        case 0x88: adc(b); break;
        case 0x89: adc(c); break;
        case 0x8A: adc(d); break;
        case 0x8B: adc(e); break;
        case 0x8C: adc(h); break;
        case 0x8D: adc(l); break;
        case 0x8E: adc(m); break;
        case 0x8F: adc(a); break;

        case 0x90: sub(b); break;
        case 0x91: sub(c); break;
        case 0x92: sub(d); break;
        case 0x93: sub(e); break;
        case 0x94: sub(h); break;
        case 0x95: sub(l); break;
        case 0x96: sub(m); break;
        case 0x97: sub(a); break;
        case 0x98: sbb(b); break;
        case 0x99: sbb(c); break;
        case 0x9A: sbb(d); break;
        case 0x9B: sbb(e); break;
        case 0x9C: sbb(h); break;
        case 0x9D: sbb(l); break;
        case 0x9E: sbb(m); break;
        case 0x9F: sbb(a); break;

        case 0xA0: ana(b); break;
        case 0xA1: ana(c); break;
        case 0xA2: ana(d); break;
        case 0xA3: ana(e); break;
        case 0xA4: ana(h); break;
        case 0xA5: ana(l); break;
        case 0xA6: ana(m); break;
        case 0xA7: ana(a); break;
        case 0xA8: xra(b); break;
        case 0xA9: xra(c); break;
        case 0xAA: xra(d); break;
        case 0xAB: xra(e); break;
        case 0xAC: xra(h); break;
        case 0xAD: xra(l); break;
        case 0xAE: xra(m); break;
        case 0xAF: xra(a); break;

        case 0xB0: ora(b); break;
        case 0xB1: ora(c); break;
        case 0xB2: ora(d); break;
        case 0xB3: ora(e); break;
        case 0xB4: ora(h); break;
        case 0xB5: ora(l); break;
        case 0xB6: ora(m); break;
        case 0xB7: ora(a); break;
        case 0xB8: cmp(b); break;
        case 0xB9: cmp(c); break;
        case 0xBA: cmp(d); break;
        case 0xBB: cmp(e); break;
        case 0xBC: cmp(h); break;
        case 0xBD: cmp(l); break;
        case 0xBE: cmp(m); break;
        case 0xBF: cmp(a); break;

        case 0xC0: ret(getZero() == 0); break;
        case 0xC1: pop(bc); break;
        case 0xC2: jmp(getZero() == 0); break;
        case 0xC3: jmp(); break;
        case 0xC4: call(getZero() == 0); break;
        case 0xC5: push(bc); break;
        case 0xC6: add(mmu.memRead(pc)); pc++; break;
        case 0xC7: rst(0); break;
        case 0xC8: ret(); break;
        case 0xC9: ret(getZero() == 1); break;
        case 0xCA: jmp(getZero() == 1); break;
        case 0xCB: jmp(); break;
        case 0xCC: call(getZero() == 1); break;
        case 0xCD: call(); break;
        case 0xCE: adc(mmu.memRead(pc)); pc++; break;
        case 0xCF: rst(1); break;

        case 0xD0: ret(getCarry() == 0); break;
        case 0xD1: pop(de); break;
        case 0xD2: jmp(getCarry() == 0); break;
        case 0xD3: out(); break;
        case 0xD4: call(getCarry() == 0); break;
        case 0xD5: push(de); break;
        case 0xD6: sub(mmu.memRead(pc)); pc++; break;
        case 0xD7: rst(2); break;
        case 0xD8: ret(); break;
        case 0xD9: ret(getCarry() == 1); break;
        case 0xDA: jmp(getCarry() == 1); break;
        case 0xDB: in(); break;
        case 0xDC: call(getCarry() == 1); break;
        case 0xDD: call(); break;
        case 0xDE: sbb(mmu.memRead(pc)); pc++; break;
        case 0xDF: rst(3); break;

        case 0xE0: ret(getParity() == 0); break;
        case 0xE1: pop(hl); break;
        case 0xE2: jmp(getParity() == 0); break;
        case 0xE3: xthl(); break;
        case 0xE4: call(getParity() == 0); break;
        case 0xE5: push(hl); break;
        case 0xE6: ana(mmu.memRead(pc)); pc++; break;
        case 0xE7: rst(4); break;
        case 0xE8: pchl(); break;
        case 0xE9: ret(getParity() == 1); break;
        case 0xEA: jmp(getParity() == 1); break;
        case 0xEB: xchg(); break;
        case 0xEC: call(getParity() == 1); break;
        case 0xED: call(); break;
        case 0xEE: xra(mmu.memRead(pc)); pc++; break;
        case 0xEF: rst(5); break;

        case 0xF0: ret(getSign() == 0); break;
        case 0xF1: pop(psw); break;
        case 0xF2: jmp(getSign() == 0); break;
        case 0xF3: di(); break;
        case 0xF4: call(getSign() == 0); break;
        case 0xF5: push(psw); break;
        case 0xF6: ora(mmu.memRead(pc)); pc++; break;
        case 0xF7: rst(6); break;
        case 0xF8: sphl(); break;
        case 0xF9: ret(getSign() == 1); break;
        case 0xFA: jmp(getSign() == 1); break;
        case 0xFB: ei(); break;
        case 0xFC: call(getSign() == 1); break;
        case 0xFD: call(); break;
        case 0xFE: cmp(mmu.memRead(pc)); pc++; break;
        case 0xFF: rst(7); break;
    }
}