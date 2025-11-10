//
//  ARM64Emitter.cpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 30/10/2025.
//

#include "ARM64Emitter.hpp"

// ARM64 instruction encoding helpers
static inline uint32_t encodeMOV(uint8_t reg, uint16_t imm) {
    // MOVZ Xd, #imm
    // encoding: |sf|1|0|1|0|1|imm16|Rd|
    // For Xd, sf=1, opcode=0b10100101, imm
    // We'll use MOVZ Xd, #imm, LSL #0 (imm16 in bits 20-5, Rd in 0-4, opcode in bits 21-31)
    return (0x52800000 | ((imm & 0xFFFF) << 5) | (reg & 0x1F));
}

static inline uint32_t encodeADD(uint8_t dst, uint8_t src1, uint8_t src2) {
    // ADD Xd, Xn, Xm
    // encoding: 0b10001011000 | Rm | sh | Rn | Rd
    // 0x8B000000 | (Rm << 16) | (Rn << 5) | Rd
    return 0x8B000000 | ((src2 & 0x1F) << 16) | ((src1 & 0x1F) << 5) | (dst & 0x1F);
}

static inline uint32_t encodeRET() {
    // RET LR: 11010110010111110000001111100000
    return 0xD65F03C0;
}

void ARM64EmitterV2::mov_reg_imm(uint8_t reg, uint16_t imm) {
    emit(encodeMOV(reg, imm));
}

void ARM64EmitterV2::add(uint8_t dst, uint8_t src1, uint8_t src2) {
    
    emit(encodeADD(dst, src1, src2));
    
}

void ARM64EmitterV2::ret() {
    
    emit(encodeRET());
    
}
