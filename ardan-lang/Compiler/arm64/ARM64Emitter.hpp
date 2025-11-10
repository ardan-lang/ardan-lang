//
//  ARM64Emitter.hpp
//  ardan-lang
//
//  Created by Chidume Nnamdi on 30/10/2025.
//

#ifndef ARM64Emitter_hpp
#define ARM64Emitter_hpp

#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <unordered_map>
#include <cstdint>
#include <iostream>

using namespace std;

class ARM64EmitterV2 {
public:
    std::vector<uint8_t> code;

    // Emit a 32-bit ARM64 instruction
    void emit(uint32_t instr) {
        code.push_back(instr & 0xFF);
        code.push_back((instr >> 8) & 0xFF);
        code.push_back((instr >> 16) & 0xFF);
        code.push_back((instr >> 24) & 0xFF);
    }

    // Add helpers for common instructions
    void mov_reg_imm(uint8_t reg, uint16_t imm);
    void add(uint8_t dst, uint8_t src1, uint8_t src2);
    void sub(uint8_t dst, uint8_t src1, uint8_t src2);
    void ret();
    
    std::vector<std::string> data;
    int addData(const std::string& str) {
        data.push_back(str);
        return 0/* address in memory or offset */;
    }
    
};

class ARM64Emitter {
public:
    ARM64Emitter() : labelCounter(0) {}

    // Emit a raw 32-bit ARM64 instruction
    void emit(uint32_t inst) {
        code.insert(code.end(),
                    reinterpret_cast<uint8_t*>(&inst),
                    reinterpret_cast<uint8_t*>(&inst) + 4);
    }

    // --- Register and immediate moves ---
    void mov_reg_imm(uint8_t reg, uint16_t imm) {
        emit(encodeMOV(reg, imm));
    }

    void mov_reg_reg(uint8_t dst, uint8_t src) {
        // ORR Xd, XZR, Xn (move Xn to Xd)
        cout << "mov x" << (int)dst << ", x" << (int)src << endl;
        emit(0xAA0003E0 | ((src & 0x1F) << 16) | ((dst & 0x1F)));
    }

    void add(uint8_t dst, uint8_t src1, uint8_t src2) {
        emit(encodeADD(dst, src1, src2));
    }

    void sub(uint8_t dst, uint8_t src1, uint8_t src2) {
        emit(encodeSUB(dst, src1, src2));
    }

    void ret() {
        cout << "ret" << '\n';
        emit(0xD65F03C0); // RET LR
    }

    void cmp_imm(uint8_t reg, int32_t imm) {
        // CMP Xreg, #imm is SUBS XZR, Xreg, #imm (immediate encoding, 12 bits)
        // SUBS XZR, Xn, #imm12
        if (imm < 0 || imm > 4095) throw std::runtime_error("Immediate too large for CMP");
        emit(0xF100001F | ((imm & 0xFFF) << 10) | ((reg & 0x1F) << 5));
    }

    void b_eq(int label) {
        pendingBranches.push_back({static_cast<int>(code.size()), label, CondEQ});
        emit(0x54000000); // BEQ placeholder, patched in resolveLabels
    }

    void b(int label) {
        pendingBranches.push_back({static_cast<int>(code.size()), label, CondAL});
        emit(0x14000000); // B placeholder, patched in resolveLabels
    }

    // ARM64 encoding for STR Xn, [Xm, #imm12]
    void str(int reg, int base, int offset) {
        // Only for offset divisible by 8 and in range [0, 32760]
        // Encoding: base 0xF9000000 | (offset/8)<<10 | reg<<5 | base
        std::cout << "str x" << reg  << ", " << base << " : " << offset << '\n';
        uint32_t instr = 0xF9000000
                       | (((offset / 8) & 0x7FF) << 10)
                       | ((base & 0x1F) << 5)
                       | (reg & 0x1F);
        code.push_back(instr);
    }

    // Load Xreg from [Xbase, #offset]
    // Mirrors the encoding of str but with opcode 0xF9400000
    // After the definition for str(int reg, int base, int offset), add the following definition for ldr:

    void ldr(int reg, int base, int offset) {
        // LDR Xn, [Xm, #imm12] (offset must be multiple of 8, in range [0, 32760])
        // Encoding: 0xF9400000 | (offset/8)<<10 | base<<5 | reg
        uint32_t instr = 0xF9400000
                       | (((offset / 8) & 0x7FF) << 10)
                       | ((base & 0x1F) << 5)
                       | (reg & 0x1F);
        code.push_back(instr);
    }

    // This mirrors str() but uses the LDR opcode (0xF9400000) instead of STR (0xF9000000).

    void ldr_global(int dstReg, int globalIndex) {
        int scratchReg = 21; // temporary register for address computation
        int offset = globalIndex * 8; // 8-byte slots

        // ADD X21, X20, #offset
        uint32_t addInstr = 0x91000000                       // ADD (immediate)
                          | ((offset & 0xFFF) << 10)         // imm12
                          | ((20 & 0x1F) << 5)               // X20 = base
                          | (scratchReg & 0x1F);             // X21 = destination
        code.push_back(addInstr);

        // LDR Xdst, [X21]
        uint32_t ldrInstr = 0xF9400000                        // base opcode for LDR 64-bit
                          | ((scratchReg & 0x1F) << 5)       // address register
                          | (dstReg & 0x1F);                 // destination register
        code.push_back(ldrInstr);

        std::cout << "ldr x" << dstReg << ", [x20 + #" << offset << "]" << std::endl;
    }

    // For global storage, you’d typically generate address in a register, then STR to [reg]
    // This stub assumes you have preallocated global addresses and store their base in a register
//    void _str_global(int reg, int global_addr_reg) {
//        // STR Xreg, [Xglobal_addr_reg]
//        // Encoding: STR Xn, [Xm, #0]
//        std::cout << "str x" << reg  << ", [" << global_addr_reg << "]" << '\n';
//
//        uint32_t instr = 0xF9000000
//                       | ((global_addr_reg & 0x1F) << 5)
//                       | (reg & 0x1F);
//        code.push_back(instr);
//    }
//
//    void str_global(int srcReg, int offsetIndex) {
//        int scratchReg = 21; // e.g., x21 for address computation
//
//        int offset = offsetIndex * 8; // 8-byte slots
//
//        // ADD X21, X20, #offset
//        uint32_t addInstr = 0x91000000                       // base opcode for ADD (immediate)
//                          | ((offset & 0xFFF) << 10)         // imm12 (limited to 12 bits)
//                          | ((20 & 0x1F) << 5)               // source register (X20)
//                          | (scratchReg & 0x1F);             // destination (X21)
//        code.push_back(addInstr);
//        std::cout << "add x" << (int)scratchReg << ", x20, #" << offset << "" << std::endl;
//
//        // STR Xsrc, [Xscratch]
//        uint32_t strInstr = 0xF9000000
//                          | ((scratchReg & 0x1F) << 5)
//                          | (srcReg & 0x1F);
//        code.push_back(strInstr);
//
//        std::cout << "str x" << srcReg << ", [x20 + #" << offset << "]" << std::endl;
//    }

    void str_global(int srcReg, int offsetIndex) {
        int scratchReg = 10; // x21 for address computation
        int offset = offsetIndex * 8; // 8-byte slots
        int base = 0;

        if (offset > 0xFFF) {
            std::cerr << "Error: offset too large for 12-bit immediate" << std::endl;
            return;
        }

        // ADD Xscratch, X0, #offset
        // Encoding: 0x91000000 | (imm12 << 10) | (Rn << 5) | Rd
        uint32_t addInstr = 0x91000000
                          | ((offset & 0xFFF) << 10)  // imm12
                          | ((base & 0x1F) << 5)         // base = x0
                          | (scratchReg & 0x1F);      // destination = x21
        emit(addInstr);
        std::cout << "add x" << scratchReg << ", x0, #" << offset << std::endl;

        // STR Xsrc, [Xscratch]
        // Encoding: 0xF9000000 | (imm12 << 10) | (Rn << 5) | Rt
        uint32_t strInstr = 0xF9000000
                          | (0 << 10)                 // imm12 = 0, offset already in x21
                          | ((scratchReg & 0x1F) << 5)  // base = x21
                          | (srcReg & 0x1F);             // source = srcReg
        emit(strInstr);
        std::cout << "str x" << srcReg << ", [x" << scratchReg << "]" << std::endl;
    }

    int genLabel() {
        return labelCounter++;
    }

    void setLabel(int label) {
        labels[label] = static_cast<int>(code.size());
    }

    void push_fp_lr() {
        // STP FP, LR, [SP, #-16]!
        emit(0xA9BF7BF0);
        // MOV FP, SP
        emit(0x910003FD);
    }

    void pop_fp_lr() {
        // LDP FP, LR, [SP], #16
        emit(0xA8C17BF0);
    }

    void blr(uint8_t reg) {
        emit(0xD63F0000 | ((reg & 0x1F) << 5));
    }

    int addData(const std::string& value) {
        int addr = static_cast<int>(dataSection.size());
        dataSection.insert(dataSection.end(), value.begin(), value.end());
        dataSection.push_back('\0');
        return addr;
    }

    // Patch branch offsets after full emission
    void resolveLabels() {
        for (const auto& br : pendingBranches) {
            int src = br.pos / 4;
            int dst = labels[br.label];
            int offset = (dst - src) & 0x3FFFFFF;
            uint32_t* inst = reinterpret_cast<uint32_t*>(&code[br.pos]);
            if (br.cond == CondAL) {
                // B
                *inst = 0x14000000 | offset;
            } else if (br.cond == CondEQ) {
                // BEQ
                *inst = 0x54000000 | ((offset << 5) & 0x03FFFFE0);
            }
        }
    }

    // Access raw code/data for output or execution
    const std::vector<uint8_t>& getCode() const { return code; }
    const std::vector<uint8_t>& getDataSection() const { return dataSection; }

private:
    enum Cond { CondAL, CondEQ };
    struct Branch {
        int pos;
        int label;
        Cond cond;
    };

    // ARM64 encoding helpers
    static inline uint32_t encodeMOV(uint8_t reg, uint16_t imm) {
        // MOVZ Xd, #imm (16-bit), LSL #0
        cout << "movz x" << (int)reg << ", " << (int)imm << '\n';
        return 0xD2800000 | ((imm & 0xFFFF) << 5) | (reg & 0x1F);
    }
    static inline uint32_t encodeADD(uint8_t dst, uint8_t src1, uint8_t src2) {
        cout << "add x" << (int)dst << ", x" << (int)src1 << ", x" << (int)src2 << '\n';
        return 0x8B000000 | ((src2 & 0x1F) << 16) | ((src1 & 0x1F) << 5) | (dst & 0x1F);
    }
    static inline uint32_t encodeSUB(uint8_t dst, uint8_t src1, uint8_t src2) {
        cout << "sub x" << (int)dst << ", x" << (int)src1 << ", x" << (int)src2 << '\n';
        return 0xCB000000 | ((src2 & 0x1F) << 16) | ((src1 & 0x1F) << 5) | (dst & 0x1F);
    }

    int labelCounter;
    std::unordered_map<int, int> labels;
    std::vector<Branch> pendingBranches;

    std::vector<uint8_t> code;
    std::vector<uint8_t> dataSection;
    
};

#endif /* ARM64Emitter_hpp */

