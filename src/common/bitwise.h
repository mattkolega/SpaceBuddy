#pragma once

#include <cstdint>

#include "types.h"

namespace Bitwise {
    // Checks if half carry occurred in 8-bit addition
    inline bool checkHalfCarryAdd(u8 operand1, u8 operand2) {
        return ((((operand1 & 0xF) + (operand2 & 0xF)) & 0x10) == 0x10);
    }

    // Checks if half carry occurred in 16-bit addition
    inline bool checkHalfCarryAdd(u16 operand1, u16 operand2) {
        return ((((operand1 & 0xFFF) + (operand2 & 0xFFF)) & 0x1000) == 0x1000);
    }

    // Checks if half carry occurred in 8-bit subtraction
    inline bool checkHalfCarrySub(u8 operand1, u8 operand2) {
        return ((((operand1 & 0xF) - (operand2 & 0xF)) & 0x10) == 0x10);
    }

    // Checks if half carry occurred in 16-bit subtraction
    inline bool checkHalfCarrySub(u16 operand1, u16 operand2) {
        return ((((operand1 & 0xFFF) - (operand2 & 0xFFF)) & 0x1000) == 0x1000);
    }

    // Gets n-th bit from an 8-bit value
    inline u8 getBitInByte(u8 value, u8 n) {
        return (value >> n) & 0b1;
    }

    // Modifies a specific bit in a 8-bit value
    inline u8 modifyBitInByte(u8 value, u8 n, u8 newVal) {
        return (value & ~(1 << n)) | ((newVal & 1) << n);
    }

    // Grabs the first 4-bits from a byte
    inline u8 getFirstNibble(u8 value) {
        return (value >> 4) & 0xF;
    }

    // Grabs the second 4-bits from a byte
    inline u8 getSecondNibble(u8 value) {
        return value & 0xF;
    }

    // Swaps the upper and lower nibbles of a byte
    inline u8 swapNibbles(u8 value) {
        return (value << 4) | (value >> 4);
    }

    // Joins two bytes together to make a 16-bit value
    inline uint16_t concatBytes(u8 low, u8 hi) {
        return (hi << 8) | low;
    }
}