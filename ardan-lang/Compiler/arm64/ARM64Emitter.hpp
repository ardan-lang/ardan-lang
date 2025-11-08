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
        emit(0xAA0003E0 | ((src & 0x1F) << 16) | ((dst & 0x1F)));
    }

    void add(uint8_t dst, uint8_t src1, uint8_t src2) {
        emit(encodeADD(dst, src1, src2));
    }

    void sub(uint8_t dst, uint8_t src1, uint8_t src2) {
        emit(encodeSUB(dst, src1, src2));
    }

    void ret() {
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
        uint32_t instr = 0xF9000000
                       | (((offset / 8) & 0x7FF) << 10)
                       | ((base & 0x1F) << 5)
                       | (reg & 0x1F);
        code.push_back(instr);
    }

    // For global storage, you’d typically generate address in a register, then STR to [reg]
    // This stub assumes you have preallocated global addresses and store their base in a register
    void str_global(int reg, int global_addr_reg) {
        // STR Xreg, [Xglobal_addr_reg]
        // Encoding: STR Xn, [Xm, #0]
        uint32_t instr = 0xF9000000
                       | ((global_addr_reg & 0x1F) << 5)
                       | (reg & 0x1F);
        code.push_back(instr);
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
        return 0xD2800000 | ((imm & 0xFFFF) << 5) | (reg & 0x1F);
    }
    static inline uint32_t encodeADD(uint8_t dst, uint8_t src1, uint8_t src2) {
        return 0x8B000000 | ((src2 & 0x1F) << 16) | ((src1 & 0x1F) << 5) | (dst & 0x1F);
    }
    static inline uint32_t encodeSUB(uint8_t dst, uint8_t src1, uint8_t src2) {
        return 0xCB000000 | ((src2 & 0x1F) << 16) | ((src1 & 0x1F) << 5) | (dst & 0x1F);
    }

    int labelCounter;
    std::unordered_map<int, int> labels;
    std::vector<Branch> pendingBranches;

    std::vector<uint8_t> code;
    std::vector<uint8_t> dataSection;
    
};

#endif /* ARM64Emitter_hpp */
