#pragma once

#include <string_view>

#include "common/types.h"

class MMU;
class IOPorts;

enum class CPUFlags : u8 {
    C = 0, // Carry
    P = 2, // Parity
    A = 4, // Aux Carry
    Z = 6, // Zero
    S = 7, // Sign
};

enum class InterruptType {
    MidFrame,
    EndFrame
};

union RegisterPair {
    u16 full;
    struct { u8 lo; u8 hi; };
};

enum class Reg8  { A, F, B, C, D, E, H, L, M };
enum class Reg16 { AF, BC, DE, HL, SP };

struct OpcodeInfo {
    std::string_view mnemonic;
    u8 cycles;
};

// Implementation of the Intel 8080
class CPU {
public:
    int cycleDelay {0};

    CPU(MMU& mmu, IOPorts& ioPorts)
        : m_mmu(mmu), m_ioPorts(ioPorts) {};

    // Executes a single opcode.
    void step();

    void triggerInterrupt(InterruptType interruptType);

private:
    MMU& m_mmu;
    IOPorts& m_ioPorts;

    RegisterPair m_af {};
    RegisterPair m_bc {};
    RegisterPair m_de {};
    RegisterPair m_hl {};
    u16          m_pc {};
    u16          m_sp {};

    bool m_isHalted          {false};
    bool m_interruptsEnabled {false};
    bool m_midFrameInterrupt {false};
    bool m_endFrameInterrupt {false};

    u8 readReg8(Reg8 reg);
    u16 readReg16(Reg16 reg);
    void writeReg8(Reg8 reg, u8 value);
    void writeReg16(Reg16 reg, u16 value);

    void pushToStack(u16 value);
    u16 popFromStack();

    u8 getCarry() const;
    u8 getParity() const;
    u8 getAuxCarry() const;
    u8 getZero() const;
    u8 getSign() const;

    void setCarry(bool value);
    void setParity(bool value);
    void setAuxCarry(bool value);
    void setZero(bool value);
    void setSign(bool value);

    void execute(u8 opcode);

    // - INSTRUCTIONS -

    // Carry bit instructions
    void cmc();
    void stc();

    // Single register instructions
    void inr(Reg8 reg);
    void dcr(Reg8 reg);
    void cma();
    void daa();

    // Data transfer instructions
    void mov(Reg8 dst, u8 src);
    void stax(Reg16 reg);
    void ldax(Reg16 reg);

    // Arithmetic instructions
    void add(u8 value);
    void adc(u8 value);
    void sub(u8 value);
    void sbb(u8 value);

    // Logical instructions
    void ana(u8 value);
    void xra(u8 value);
    void ora(u8 value);
    void cmp(u8 value);

    // Rotation instructions
    void rlc();
    void rrc();
    void ral();
    void rar();

    // Register pair instructions
    void push(Reg16 reg);
    void pop(Reg16 reg);
    void dad(Reg16 reg);
    void inx(Reg16 reg);
    void dcx(Reg16 reg);
    void xchg();
    void xthl();
    void sphl();

    // Immediate instructions
    void lxi(Reg16 reg, u16 immediate);

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
    void in(u8 portNum);
    void out(u8 portNum);

    // Halts CPU
    void hlt();
};
