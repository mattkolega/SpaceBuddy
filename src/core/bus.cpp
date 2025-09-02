#include "bus.h"

#include <common/types.h>

Bus::Bus() : cpu(*this) {};

void Bus::init() {
    mmu.loadRom();
}

void Bus::run() {
    cpu.step();
}

u8 Bus::memRead(u16 address) const {
    return mmu.read(address);
}

void Bus::memWrite(u16 address, u8 value) {
    mmu.write(address, value);
}

u8& Bus::getMemRef(u16 address) {
    return mmu.getRef(address);
}

const std::array<u8, 1024 * 7>& Bus::getFrameBuffer() const {
    return mmu.getFrameBuffer();
};

u8 Bus::in(u8 portNum) const {
    switch (portNum) {
        case 1:
            return port1;
        case 2:
            return port2;
        case 3:
            return ((shiftRegister << shiftOffset) >> 8) & 0xFF;
        default:
            return 0;
    }
}

void Bus::out(u8 portNum, u8 value) {
    switch (portNum) {
        case 2:
            shiftOffset = value;
            break;
        case 3:
            port3 = value;
            break;
        case 4:
            shiftRegister >>= 8;
            shiftRegister |= (value << 8);
            break;
        case 5:
            port5 = value;
            break;
        default:
            break;
    }
}

void Bus::triggerMidFrameInterrupt() {
    cpu.triggerInterrupt(InterruptType::MidFrame);
}

void Bus::triggerEndFrameInterrupt() {
    cpu.triggerInterrupt(InterruptType::EndFrame);
}