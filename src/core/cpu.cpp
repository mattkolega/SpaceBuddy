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