#pragma once

#include <string_view>

#include <common/bitwise.h>
#include <common/types.h>

class Bus;

enum class FlagBits : u8 {
    C = 0, // Carry
    P = 2, // Parity
    A = 4, // Aux Carry
    Z = 6, // Zero
    S = 7, // Sign
};

class RegisterPair {
public:
    u8 hi {};
    u8 lo {};

    RegisterPair() = default;
    RegisterPair(u16 value) : hi(value >> 8), lo(value & 0xFF) {};

    u16 get() const;
    void set(u16 value);
};

struct opcodeInfo {
    std::string_view mnemonic;
    u8 cycles;
};

class CPU {
public:
    CPU() = delete;
    CPU(Bus& bus);

    // Executes a single opcode.
    void step();
private:
    Bus& bus;

    RegisterPair psw {};
    RegisterPair bc  {};
    RegisterPair de  {};
    RegisterPair hl  {};

    u16 pc {};
    u16 sp {};

    u8& a = psw.hi;
    u8& f = psw.lo;
    u8& b = bc.hi;
    u8& c = bc.lo;
    u8& d = de.hi;
    u8& e = de.lo;
    u8& h = hl.hi;
    u8& l = hl.lo;

    u8& m();

    bool interruptsEnabled { false };

    void pushToStack(u16 value);
    u16 popFromStack();

    u8 getCarry() const;
    void setCarry(bool value);
    u8 getParity() const;
    void setParity(bool value);
    u8 getAuxCarry() const;
    void setAuxCarry(bool value);
    u8 getZero() const;
    void setZero(bool value);
    u8 getSign() const;
    void setSign(bool value);

    void execute();

    /* INSTRUCTIONS */

    // Carry bit instructions
    void cmc();
    void stc();

    // Single register instructions
    void inr(u8& reg);
    void dcr(u8& reg);
    void cma();
    void daa();

    // Data transfer instructions
    void mov(u8& dst, u8 src);
    void stax(u16 reg);
    void ldax(u16 reg);

    // Arithmetic instructions
    void add(u8 reg);
    void adc(u8 reg);
    void sub(u8 reg);
    void sbb(u8 reg);

    // Logical instructions
    void ana(u8 reg);
    void xra(u8 reg);
    void ora(u8 reg);
    void cmp(u8 reg);

    // Rotation instructions
    void rlc();
    void rrc();
    void ral();
    void rar();

    // Register pair instructions
    void push(RegisterPair reg);
    void pop(RegisterPair& reg);
    void dad(u16 reg);
    void inx(RegisterPair& reg);
    void inx(u16& reg);
    void dcx(RegisterPair& reg);
    void dcx(u16& reg);
    void xchg();
    void xthl();
    void sphl();

    // Immediate instructions
    void lxi(RegisterPair& reg, u16 immediate);
    void lxi(u16& reg, u16 immediate);

    // Direct addressing instructions
    void sta(u16 address);
    void lda(u16 address);
    void shld(u16 address);
    void lhld(u16 address);

    // Jump instructions
    void pchl();
    void jmp(bool condition);

    // Call subroutine instructions
    void call(bool condition);
    void ret(bool condition);
    void rst(u8 data);

    // Interrupt instructions
    void ei();
    void di();

    // Input/output instructions
    void in();
    void out();

    void hlt();
};