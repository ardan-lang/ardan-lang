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

#define BASE 19
#define BYTE_SIZE 1

using namespace std;

enum Tag : uint8_t {
    TAG_NUMBER        = 0b001,
    TAG_STRING        = 0b000,
    TAG_BOOLEAN       = 0b010,
    TAG_UNDEFINED     = 0b011,
    TAG_OBJECT        = 0b100,
    TAG_NULL          = 0b101,
    TAG_ARRAY         = 0b110,
    TAG_FUNCTION      = 0b111,
    TAG_SYMBOL        = 0b1000,
    TAG_BIGINT        = 0b1001,
    TAG_DATE          = 0b1010,
    TAG_REGEXP        = 0b1011,
    TAG_CLASS         = 0b1100,
    TAG_ERROR         = 0b1101,
    TAG_MAP           = 0b1110,
    TAG_SET           = 0b1111,
    TAG_PROMISE       = 0b10000,
    TAG_WEAKMAP       = 0b10001,
    TAG_WEAKSET       = 0b10010,
    TAG_TYPEDARRAY    = 0b10011,
    TAG_BUFFER        = 0b10100,
    TAG_ITERATOR      = 0b10101,
    TAG_GENERATOR     = 0b10110,
    TAG_PROXY         = 0b10111,
    TAG_ENV           = 0b11000,
};

struct ValueTag {
    uint64_t raw;
};

static_assert(sizeof(ValueTag) == 8);

class ARM64Emitter {
public:
    ARM64Emitter() : labelCounter(0) {}

    // Emit a raw 32-bit ARM64 instruction
    void emit(uint32_t inst) {
        code.insert(code.end(),
                    reinterpret_cast<uint8_t*>(&inst),
                    reinterpret_cast<uint8_t*>(&inst) + 4);
    }

    // ---- Arithmetic ----
    
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
    
    static inline uint32_t encodeDIV(uint8_t dst, uint8_t src1, uint8_t src2) {
        cout << "div x" << (int)dst << ", x" << (int)src1 << ", x" << (int)src2 << '\n';
        return 0x9AC00800 | ((src2 & 0x1F) << 16) | ((src1 & 0x1F) << 5) | (dst & 0x1F);
    }
    
    static inline uint32_t encodeMUL(uint8_t dst, uint8_t src1, uint8_t src2) {
        cout << "mul x" << (int)dst << ", x" << (int)src1 << ", x" << (int)src2 << '\n';
        return 0x9B007C00 | ((src2 & 0x1F) << 16) | ((src1 & 0x1F) << 5) | (dst & 0x1F);
    }

    // ---- End Arithmetic ----

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
        cout << "cmp x" << (int)reg << ", #" << (int)imm << '\n';

    }
    
    void cmp_reg_reg(int reg1, int reg2) {
        // ARMv7-A CMP encoding (A32)
        //  cond(31-28) = 1110 (AL)
        //  bits 27-26 = 00 (data processing)
        //  opcode = 1010 (CMP)
        //  S = 1
        //  Rn = reg1
        //  Operand2 = reg2
        uint32_t cond = 0b1110 << 28;
        uint32_t opcode = cond | (0b00 << 26);      // data processing
        opcode |= (0b1010 << 21);                   // opcode = CMP
        opcode |= (1 << 20);                        // S bit = 1 (update flags)
        opcode |= (reg1 & 0xF) << 16;              // Rn
        opcode |= (reg2 & 0xF);                     // Operand2 = Rm (no shift)
        emit(opcode);
        cout << "cmp x" << (int)reg1 << ", x" << (int)reg2 << '\n';

    }
    
    // ------------- LDRB basic (no offset) --------------
    void ldrb(int destReg, int baseReg) {
        // LDRB Rt, [Rn]  (pre-indexed, no offset)
        uint32_t cond = 0b1110 << 28; // AL
        uint32_t opcode = cond | (0b01 << 26); // Load/store class
        opcode |= (1 << 24); // P = pre-indexed
        opcode |= (1 << 23); // U = add offset
        opcode |= (1 << 22); // B = byte
        opcode |= (0 << 21); // W = no writeback
        opcode |= (1 << 20); // L = load
        opcode |= (baseReg & 0xF) << 16; // Rn
        opcode |= (destReg & 0xF) << 12; // Rt
        opcode |= 0; // imm12 = 0
        emit(opcode);
    }

    // ------------- LDRB with immediate offset (imm12 * 8? or just imm12) --------------
    void ldrb_offset(int destReg, int baseReg, int offset) {
        uint32_t cond = 0b1110 << 28;
        uint32_t opcode = cond | (0b01 << 26);
        opcode |= (1 << 24); // P = pre-indexed
        opcode |= (1 << 23); // U = add offset
        opcode |= (1 << 22); // B = byte
        opcode |= (0 << 21); // W = no writeback
        opcode |= (1 << 20); // L = load
        opcode |= (baseReg & 0xF) << 16;
        opcode |= (destReg & 0xF) << 12;
        opcode |= offset & 0xFFF; // 12-bit offset
        cout << "ldrb x" << destReg << ", [x" << baseReg << ", #" << offset << "]" << endl;
        emit(opcode);
    }

    // ------------- LDRB with register offset --------------
    void ldrb_reg_reg(int destReg, int baseReg, int offsetReg) {
        uint32_t cond = 0b1110 << 28;
        uint32_t opcode = cond | (0b01 << 26);
        opcode |= (1 << 25); // I = 1 -> register offset
        opcode |= (1 << 24); // P = pre-indexed
        opcode |= (1 << 23); // U = add
        opcode |= (1 << 22); // B = byte
        opcode |= (0 << 21); // W = no writeback
        opcode |= (1 << 20); // L = load
        opcode |= (baseReg & 0xF) << 16;
        opcode |= (destReg & 0xF) << 12;
        opcode |= (offsetReg & 0xF); // Rm in lower 4 bits (simplified, no shift)
        emit(opcode);
    }

    // ------------- LDRB with post-increment --------------
    void ldrb_increment(int destReg, int baseReg, int increment) {
        uint32_t cond = 0b1110 << 28;
        uint32_t opcode = cond | (0b01 << 26);
        opcode |= (0 << 24); // P = 0 -> post-index
        opcode |= (1 << 23); // U = add
        opcode |= (1 << 22); // B = byte
        opcode |= (1 << 21); // W = writeback (mandatory for post-index)
        opcode |= (1 << 20); // L = load
        opcode |= (baseReg & 0xF) << 16;
        opcode |= (destReg & 0xF) << 12;
        opcode |= increment & 0xFFF; // imm12
        emit(opcode);
    }

    // ------------- LDRB with pre-increment (writeback) --------------
    void ldrb_pre_increment(int destReg, int baseReg, int increment) {
        uint32_t cond = 0b1110 << 28;
        uint32_t opcode = cond | (0b01 << 26);
        opcode |= (1 << 24); // P = 1 -> pre-index
        opcode |= (1 << 23); // U = add
        opcode |= (1 << 22); // B = byte
        opcode |= (1 << 21); // W = writeback
        opcode |= (1 << 20); // L = load
        opcode |= (baseReg & 0xF) << 16;
        opcode |= (destReg & 0xF) << 12;
        opcode |= increment & 0xFFF; // imm12
        emit(opcode);
    }

    // shiftType: 0 = LSL, 1 = LSR, 2 = ASR, 3 = ROR
    void ldrb_reg_reg_shift(int destReg, int baseReg, int offsetReg, int shiftType = 0, int shiftAmount = 0) {
        uint32_t cond = 0b1110 << 28;
        uint32_t opcode = cond | (0b01 << 26); // Load/store class
        opcode |= (1 << 25);  // I = 1 -> register offset
        opcode |= (1 << 24);  // P = pre-index
        opcode |= (1 << 23);  // U = add offset
        opcode |= (1 << 22);  // B = byte
        opcode |= (0 << 21);  // W = no writeback
        opcode |= (1 << 20);  // L = load

        opcode |= (baseReg & 0xF) << 16;  // Rn
        opcode |= (destReg & 0xF) << 12;  // Rt

        // Build shift encoding
        uint32_t shift = ((shiftAmount & 0x1F) << 7) | ((shiftType & 0x3) << 5);
        opcode |= shift;                 // bits 11–5
        opcode |= (offsetReg & 0xF);     // bits 3–0: Rm

        emit(opcode);
    }

    // LDR Xt, [Xn, #imm]
    // imm is a 12-bit *scaled* offset (scale = 8 bytes for 64-bit)
    uint32_t ldr_reg_reg_imm(uint8_t Rt, uint8_t Rn, uint16_t imm12) {
        // size=11, V=0, opc=01 → LDR 64-bit unsigned immediate
        uint32_t op = 0;
        op |= (0b11    << 30);     // size
        op |= (0        << 29);    // V
        op |= (0b01     << 27);    // opc
        op |= (imm12    << 10);    // imm12 (scaled)
        op |= (Rn       << 5);
        op |= (Rt       << 0);
        return op;
    }
    
    // Helper to calculate bit length (similar to Python's int.bit_length())
    // Returns the number of bits required to represent n.
    // e.g., 7 (111) -> 3, 255 (11111111) -> 8
    int get_bit_width(uint64_t n) {
        if (n == 0) return 0;
        int bits = 0;
        while (n > 0) {
            n >>= 1;
            bits++;
        }
        return bits;
    }

    /**
     * Encodes an ARM64 'AND (immediate)' instruction for 64-bit registers (X regs).
     * * Assembly: AND Xd, Xn, #imm
     * * @param rd_idx Destination register index (0-31).
     * @param rn_idx Source register index (0-31).
     * @param imm    The integer immediate. MUST be a valid logical bitmask.
     * (e.g., 0x7, 0xFF).
     * @return       The 32-bit machine code (little endian integer).
     */
    void and_reg_reg_imm(int rd_idx, int rn_idx, uint64_t imm) {
        // --- 1. Validation ---
        if (rd_idx < 0 || rd_idx > 31 || rn_idx < 0 || rn_idx > 31) {
            throw std::invalid_argument("Registers must be between 0 and 31");
        }

        if (imm == 0 || imm == UINT64_MAX) { // Check basic bounds/empty
            throw std::invalid_argument("Immediate must be > 0 and < 2^64 - 1 for this encoder");
        }

        // Check if immediate is a valid contiguous mask (2^k - 1)
        // This checks if adding 1 makes it a power of 2
        if ((imm & (imm + 1)) != 0) {
            throw std::invalid_argument("Immediate is complex (rotated/repeating). This simple encoder only supports standard contiguous bitmasks.");
        }

        // --- 2. Calculate Logical Immediate Fields (N, immr, imms) ---
        // For 64-bit operations:
        // N = 1
        // immr = 0 (No rotation needed for standard masks starting at bit 0)
        // imms = Highest bit set index.
        
        uint32_t N = 1;
        uint32_t immr = 0;
        
        // Calculate bit length and subtract 1 to get the highest bit index
        // Example: imm=7 (binary 111). width=3. imms = 2.
        uint32_t imms = get_bit_width(imm) - 1;

        // --- 3. Construct the 32-bit Instruction ---
        // Format for AND (immediate, 64-bit):
        // | 31 | 30 29 | 28 ... 23 | 22 | 21 ... 16 | 15 ... 10 | 9 ... 5 | 4 ... 0 |
        // | sf |  opc  |  100100   | N  |   immr    |   imms    |   Rn    |   Rd    |
        
        uint32_t sf = 1;
        uint32_t opc = 0;
        uint32_t magic = 0b100100; // 36 decimal

        uint32_t instruction = 0;
        
        instruction |= (sf << 31);
        instruction |= (opc << 29);
        instruction |= (magic << 23);
        instruction |= (N << 22);
        instruction |= (immr << 16);
        instruction |= (imms << 10);
        instruction |= (static_cast<uint32_t>(rn_idx) << 5);
        instruction |= (static_cast<uint32_t>(rd_idx) << 0);
        
        emit(instruction);

        cout << "and x" << (int)rd_idx << ", x" << (int)rn_idx << ", #" << (int)imm << '\n';

    }
    
    /**
     * Encodes an ARM64 'STR Xd, [SP, #imm]!' instruction (Store Register, Pre-Indexed).
     * Assembly: STR <Xt>, [<Xn|SP>, #<simm9>]!
     * @param rt_idx Source register index (0-31).
     * @param rn_idx Base/Writeback register index (SP=31 or X0-30).
     * @param simm   The signed 9-bit immediate offset (-256 to 255).
     * @return       The 32-bit machine code (little endian integer).
     */
    void str_pre_indexed(int rt_idx, int rn_idx, int simm) {
        // --- 1. Validation ---
        if (rt_idx < 0 || rt_idx > 31 || rn_idx < 0 || rn_idx > 31) {
            throw std::invalid_argument("Registers must be between 0 and 31");
        }
        if (simm < -256 || simm > 255) {
            throw std::invalid_argument("Immediate for pre-indexed addressing must be between -256 and 255");
        }

        // --- 2. Construct Fields ---
        // Format: Load/Store Register (Immediate Pre-indexed)
        // | 31 30 | 29 24 | 23 22 | 21 12 | 11 10 | 9 5 | 4 0 |
        // | size  | 111110 | opc  | simm9 |  01   | Rn  | Rt  |
        
        uint32_t size = 0b11;   // 64-bit (8-byte)
        uint32_t fixed_24_29 = 0b111110;
        uint32_t opc = 0b00;    // Store
        uint32_t op2 = 0b01;    // Pre-indexed addressing mode (Writeback)

        // Convert signed immediate to 9-bit unsigned two's complement value
        uint32_t simm9 = (simm < 0) ? (0x200 + simm) : simm;

        uint32_t instruction = 0;
        instruction |= (size << 30);
        instruction |= (fixed_24_29 << 24);
        instruction |= (opc << 22);
        instruction |= (simm9 << 12);
        instruction |= (op2 << 10);
        instruction |= (static_cast<uint32_t>(rn_idx) << 5);
        instruction |= (static_cast<uint32_t>(rt_idx) << 0);

        emit(instruction);
    }

    /**
     * Encodes an ARM64 'ORR (immediate)' instruction for 64-bit registers (X regs).
     * Assembly: ORR Xd, Xn, #imm
     * @param rd_idx Destination register index (0-31).
     * @param rn_idx Source register index (0-31).
     * @param imm    The integer immediate. MUST be a valid logical bitmask.
     * @return       The 32-bit machine code (little endian integer).
     */
    void orr_reg_reg_imm(int rd_idx, int rn_idx, uint64_t imm) {
        if (rd_idx < 0 || rd_idx > 31 || rn_idx < 0 || rn_idx > 31) {
            throw std::invalid_argument("Registers must be between 0 and 31");
        }

        if (imm == 0 || imm == UINT64_MAX) {
            throw std::invalid_argument("Immediate must be > 0 and < 2^64 - 1 for this encoder");
        }

        // Check if immediate is a valid contiguous mask (2^k - 1)
        if ((imm & (imm + 1)) != 0) {
            throw std::invalid_argument("Immediate is complex. This simple encoder only supports contiguous bitmasks (e.g. 0x7, 0xFF).");
        }

        uint32_t N = 1;
        uint32_t immr = 0;
        uint32_t imms = get_bit_width(imm) - 1;

        uint32_t sf = 1;
        uint32_t opc = 0b01; // 1 (ORR)
        uint32_t magic = 0b100100; // 36 decimal (Logical Immediate)

        uint32_t instruction = 0;
        instruction |= (sf << 31);
        instruction |= (opc << 29);
        instruction |= (magic << 23);
        instruction |= (N << 22);
        instruction |= (immr << 16);
        instruction |= (imms << 10);
        instruction |= (static_cast<uint32_t>(rn_idx) << 5);
        instruction |= (static_cast<uint32_t>(rd_idx) << 0);

        emit(instruction);
    }

    /**
     * Encodes an ARM64 'ORR (Shifted Register)' instruction for 64-bit registers (X regs).
     * Assembly: ORR Xd, Xn, Xm
     * This uses the unshifted version of the Data Processing (2 Source) format.
     * @param rd_idx Destination register index (0-31).
     * @param rn_idx Source register index (0-31).
     * @param rm_idx Second source register index (0-31).
     * @return       The 32-bit machine code (little endian integer).
     */
    void orr_reg_reg_reg(int rd_idx, int rn_idx, int rm_idx) {
        // --- 1. Validation ---
        if (rd_idx < 0 || rd_idx > 31 || rn_idx < 0 || rn_idx > 31 || rm_idx < 0 || rm_idx > 31) {
            throw std::invalid_argument("Registers must be between 0 and 31");
        }

        // --- 2. Construct Fields ---
        // Format: Data Processing (2 Source) - Shifted Register
        // | 31 | 30 29 | 28 ... 24 | 23 22 | 21 ... 16 | 15 14 | 13 12 | 11 10 | 9 ... 5 | 4 ... 0 |
        // | sf | 00(opc) | 010100  | 00(op2) |   Rm    | 00(imm) | shift | 00(imm) |   Rn    |   Rd    |
        
        uint32_t sf = 1;            // 64-bit
        uint32_t opc = 0b10;        // 2 (ORR) - Note: ORR opc is 10 for shifted register
        uint32_t fixed_24_28 = 0b01010; // Data Processing - Shifted Register group
        uint32_t imm3_fixed = 0b000;
        uint32_t N = 0;
        uint32_t fixed_10_15 = 0b000000; // 00(imm) + shift(00) + 00(imm)
        
        // Core fields
        uint32_t fixed_30_29 = 0b01; // This group uses 01
        uint32_t Rm = static_cast<uint32_t>(rm_idx);
        uint32_t Rn = static_cast<uint32_t>(rn_idx);
        uint32_t Rd = static_cast<uint32_t>(rd_idx);

        // Encoding:
        // Bits 31=sf, 30:29=01, 28:24=10101 (op0=01, opc=0, op2=01)
        // The ORR encoding is: 0b10010101000...
        
        uint32_t magic_top = 0b1010100; // Bits 31:24 = 10010101 (sf:opc:fixed_24_28)

        uint32_t instruction = 0;
        
        // ORR is part of the Data Processing (2 Source) group.
        // Fixed pattern: 1001010100 opc(21:20)=10 shift(15:14)=00 imm6(13:10)=0000
        // ORR: 0b101010 (Bits 31, 30-24)
        instruction |= (sf << 31);
        instruction |= (0b01 << 29); // op0 (top two bits of 30:29-24 block)
        instruction |= (0b01010 << 24); // op2 + top of fixed field
        instruction |= (0b10 << 21); // opc for ORR
        instruction |= (Rm << 16);
        instruction |= (0b00 << 14); // Shift LSL
        instruction |= (0b000000 << 10); // imm6 = 0
        instruction |= (Rn << 5);
        instruction |= (Rd << 0);

        emit(instruction);
    }

    /**
     * Encodes an ARM64 'LSR (immediate)' instruction for 64-bit registers (X regs).
     * Note: LSR is an alias for UBFM (Unsigned Bitfield Move).
     * Alias: LSR <Xd>, <Xn>, #<shift>  =>  UBFM <Xd>, <Xn>, #<shift>, #63
     * * @param rd_idx Destination register index (0-31).
     * @param rn_idx Source register index (0-31).
     * @param shift  The number of bits to shift right (0-63).
     * @return       The 32-bit machine code (little endian integer).
     */
    void lsr_reg_reg_imm(int rd_idx, int rn_idx, int shift) {
        // --- 1. Validation ---
        if (rd_idx < 0 || rd_idx > 31 || rn_idx < 0 || rn_idx > 31) {
            throw std::invalid_argument("Registers must be between 0 and 31");
        }
        
        if (shift < 0 || shift > 63) {
            throw std::invalid_argument("Shift amount must be between 0 and 63 for 64-bit registers");
        }

        // --- 2. Construct Fields ---
        // Encoding for UBFM (64-bit):
        // | 31 | 30 29 | 28 ... 23 | 22 | 21 ... 16 | 15 ... 10 | 9 ... 5 | 4 ... 0 |
        // | sf |  opc  |  100110   | N  |   immr    |   imms    |   Rn    |   Rd    |
        
        // Standard mapping for LSR Xd, Xn, #shift:
        // immr = shift
        // imms = 63 (This keeps the bottom bits of the rotated result)
        
        uint32_t sf = 1;            // 64-bit
        uint32_t opc = 0b10;        // 2 (UBFM / Bitfield)
        uint32_t magic = 0b100110;  // 38 decimal (Bitfield)
        uint32_t N = 1;             // 64-bit
        uint32_t immr = static_cast<uint32_t>(shift);
        uint32_t imms = 63;

        uint32_t instruction = 0;
        
        instruction |= (sf << 31);
        instruction |= (opc << 29);
        instruction |= (magic << 23);
        instruction |= (N << 22);
        instruction |= (immr << 16);
        instruction |= (imms << 10);
        instruction |= (static_cast<uint32_t>(rn_idx) << 5);
        instruction |= (static_cast<uint32_t>(rd_idx) << 0);

        cout << "lsr x" << (int)rd_idx << ", x" << (int)rn_idx << ", #" << (int)shift << '\n';

        emit(instruction);
        
    }

    /**
     * Encodes an ARM64 'LSLV' (Logical Shift Left Variable) instruction.
     * Assembly: LSL Xd, Xn, Xm
     * The shift amount is taken from the least significant 6 bits of Xm.
     * @param rd_idx Destination register index (0-31).
     * @param rn_idx Source register index (0-31).
     * @param rm_idx Register index holding the shift amount (0-31).
     * @return       The 32-bit machine code (little endian integer).
     */
    void lsl_reg_reg_reg(int rd_idx, int rn_idx, int rm_idx) {
        // --- 1. Validation ---
        if (rd_idx < 0 || rd_idx > 31 || rn_idx < 0 || rn_idx > 31 || rm_idx < 0 || rm_idx > 31) {
            throw std::invalid_argument("Registers must be between 0 and 31");
        }

        // --- 2. Construct Fields ---
        // This instruction belongs to the Data Processing (3 Source) group: Shift (variable)
        // | 31 | 30-29 | 28-24 | 23-21 | 20-16 | 15-10 | 9-5 | 4-0 |
        // | sf |  11   | 01011 | op2=000 |  Rm   | opcode=000010 |  Rn |  Rd |
        
        uint32_t sf = 1;            // 64-bit
        uint32_t fixed_opc_29_30 = 0b11; // Bits 30 & 29 are 11
        uint32_t fixed_24_28 = 0b01011;  // Bits 28-24
        uint32_t op2 = 0b000;         // Bits 23-21
        uint32_t lslv_opcode = 0b000010; // Bits 15-10 for LSLV

        uint32_t instruction = 0;
        
        instruction |= (sf << 31);
        instruction |= (fixed_opc_29_30 << 29);
        instruction |= (fixed_24_28 << 24);
        instruction |= (op2 << 21);
        instruction |= (static_cast<uint32_t>(rm_idx) << 16);
        instruction |= (lslv_opcode << 10);
        instruction |= (static_cast<uint32_t>(rn_idx) << 5);
        instruction |= (static_cast<uint32_t>(rd_idx) << 0);

        emit(instruction);
    }

    /**
     * Encodes an ARM64 'LSL (immediate)' instruction for 64-bit registers (X regs).
     * Note: LSL is an alias for UBFM (Unsigned Bitfield Move).
     * Alias: LSL <Xd>, <Xn>, #<shift>  =>  UBFM <Xd>, <Xn>, #(64-shift), #(63-shift)
     * * @param rd_idx Destination register index (0-31).
     * @param rn_idx Source register index (0-31).
     * @param shift  The number of bits to shift left (0-63).
     * @return       The 32-bit machine code (little endian integer).
     */
    void lsl_reg_reg_imm(int rd_idx, int rn_idx, int shift) {
        // --- 1. Validation ---
        if (rd_idx < 0 || rd_idx > 31 || rn_idx < 0 || rn_idx > 31) {
            throw std::invalid_argument("Registers must be between 0 and 31");
        }
        
        if (shift < 0 || shift > 63) {
            throw std::invalid_argument("Shift amount must be between 0 and 63 for 64-bit registers");
        }

        // --- 2. Construct Fields (UBFM format) ---
        // The LSL alias requires:
        // immr = 64 - shift
        // imms = 63 - shift
        
        uint32_t sf = 1;            // 64-bit
        uint32_t opc = 0b10;        // 2 (UBFM / Bitfield)
        uint32_t magic = 0b100110;  // 38 decimal (Bitfield)
        uint32_t N = 1;             // 64-bit
        uint32_t immr = static_cast<uint32_t>(64 - shift);
        uint32_t imms = static_cast<uint32_t>(63 - shift);

        uint32_t instruction = 0;
        
        instruction |= (sf << 31);
        instruction |= (opc << 29);
        instruction |= (magic << 23);
        instruction |= (N << 22);
        instruction |= (immr << 16);
        instruction |= (imms << 10);
        instruction |= (static_cast<uint32_t>(rn_idx) << 5);
        instruction |= (static_cast<uint32_t>(rd_idx) << 0);

        emit(instruction);
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
        // code.push_back(instr);
        emit(instr);
        cout << "str x" << (int)reg << ", " << (int)base << ", " << (int)offset << '\n';
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

        cout << "ldr x" << (int)reg << ", x" << (int)base << ", #" << (int)offset << '\n';
        
        emit(instr);
    }

    // This mirrors str() but uses the LDR opcode (0xF9400000) instead of STR (0xF9000000).

    void ldr_global(int destReg, int offsetIndex) {
        int scratchReg = 9; // x10 for address computation
        int offset = offsetIndex * BYTE_SIZE; // 8-byte slots

        if (offset > 0xFFF) {
            std::cerr << "Error: offset too large for 12-bit immediate" << std::endl;
            return;
        }

        // ADD Xscratch, X0, #offset
        // Encoding: 0x91000000 | (imm12 << 10) | (Rn << 5) | Rd
        uint32_t addInstr = 0x91000000
                          | ((offset & 0xFFF) << 10)    // imm12
                          | ((BASE & 0x1F) << 5)        // base = x0
                          | (scratchReg & 0x1F);        // destination = x10
        emit(addInstr);
        std::cout << "add x" << scratchReg << ", x" << BASE << ", #" << offset << std::endl;

        // LDR Xdest, [Xscratch]
        // Encoding: 0xF9400000 | (imm12 << 10) | (Rn << 5) | Rt
        uint32_t ldrInstr = 0xF9400000
                          | (0 << 10)                   // imm12 = 0, offset already in x10
                          | ((scratchReg & 0x1F) << 5)  // base = x10
                          | (destReg & 0x1F);           // destination = destReg
        emit(ldrInstr);
        std::cout << "ldr x" << destReg << ", [x" << scratchReg << "]" << std::endl;
    }
    
    void ldr_global_reg_reg(int destReg, int Reg) {
        uint32_t ldrInstr = 0xF9400000
                          | (0 << 10)                   // imm12 = 0, offset already in x10
                          | ((Reg & 0x1F) << 5)  // base = x10
                          | (destReg & 0x1F);           // destination = destReg
        emit(ldrInstr);
        std::cout << "ldr x" << destReg << ", [x" << Reg << "]" << std::endl;

    }
    
    // for data 8 bytes long.
    void calc_abs_addr_mov_reg_offset(int reg, int offsetIndex) {
        
        int scratchReg = 9; // x21 for address computation
        int offset = offsetIndex * BYTE_SIZE; // 8-byte slots

        if (offset > 0xFFF) {
            std::cerr << "Error: offset too large for 12-bit immediate" << std::endl;
            return;
        }

        // ADD Xscratch, X0, #offset
        // Encoding: 0x91000000 | (imm12 << 10) | (Rn << 5) | Rd
        uint32_t addInstr = 0x91000000
                          | ((offset & 0xFFF) << 10)  // imm12
                          | ((BASE & 0x1F) << 5)         // base = x0
                          | (scratchReg & 0x1F);      // destination = x21
        emit(addInstr);
        std::cout << "add x" << scratchReg << ", x" << BASE << ", #" << offset << std::endl;
        
        mov_reg_reg(reg, scratchReg);

    }

    // For global storage, you’d typically generate address in a register, then STR to [reg]
    // This stub assumes you have preallocated global addresses and store their base in a register

    void str_global(int srcReg, int offsetIndex) {
        int scratchReg = 9; // x21 for address computation
        int offset = offsetIndex * BYTE_SIZE; // 8-byte slots

        if (offset > 0xFFF) {
            std::cerr << "Error: offset too large for 12-bit immediate" << std::endl;
            return;
        }

        // ADD Xscratch, X0, #offset
        // Encoding: 0x91000000 | (imm12 << 10) | (Rn << 5) | Rd
        uint32_t addInstr = 0x91000000
                          | ((offset & 0xFFF) << 10)  // imm12
                          | ((BASE & 0x1F) << 5)         // base = x0
                          | (scratchReg & 0x1F);      // destination = x21
        emit(addInstr);
        std::cout << "add x" << scratchReg << ", x" << BASE << ", #" << offset << std::endl;

        // STR Xsrc, [Xscratch]
        // Encoding: 0xF9000000 | (imm12 << 10) | (Rn << 5) | Rt
        uint32_t strInstr = 0xF9000000
                          | (0 << 10)                 // imm12 = 0, offset already in x21
                          | ((scratchReg & 0x1F) << 5)  // base = x21
                          | (srcReg & 0x1F);             // source = srcReg
        emit(strInstr);
        std::cout << "str x" << srcReg << ", [x" << scratchReg << "]" << std::endl;
    }
    
    void strb(int srcReg, int baseReg, int offset) {
        // STRB Wt, [Xn, #imm12]
        // srcReg: value to store (low byte), baseReg: address, offset: byte offset
        if (offset < 0 || offset > 4095) {
            std::cerr << "strb: offset out of range (0..4095): " << offset << std::endl;
            return;
        }
        uint32_t instr = 0x39000000
                       | ((offset & 0xFFF) << 10)
                       | ((baseReg & 0x1F) << 5)
                       | (srcReg & 0x1F);
        emit(instr);
        std::cout << "strb w" << srcReg << ", [x" << baseReg << ", #" << offset << "]" << std::endl;
    }

    void push_fp_lr() {
        // STP FP, LR, [SP, #-16]!
        cout << "stp x29, x30, [sp, #-16]!" << endl;
        // emit(0xA9BF7BF0);
        emit(0xA9BF7BFD);

        // MOV FP, SP
        cout << "mov x29, sp" << endl;
        // emit(0x910003FD);
        emit(0xAA1F03FD);
    }

    void pop_fp_lr() {
        // LDP FP, LR, [SP], #16
        cout << "ldp x29, x30, [sp], #16" << endl;
        //emit(0xA8C17BF0);
        emit(0xA8C17BFD);
    }

    void blr(uint8_t reg) {
        cout << "blr x" << (int)reg << endl;
        emit(0xD63F0000 | ((reg & 0x1F) << 5));
    }

    // Load a 64-bit absolute address or constant into a register
    void mov_abs(uint8_t reg, uint64_t value) {
        uint16_t imm16[4] = {
            static_cast<uint16_t>((value >> 0) & 0xFFFF),
            static_cast<uint16_t>((value >> 16) & 0xFFFF),
            static_cast<uint16_t>((value >> 32) & 0xFFFF),
            static_cast<uint16_t>((value >> 48) & 0xFFFF)
        };

        // MOVZ Xd, imm16, LSL #0
        uint32_t movz = 0xD2800000 | (imm16[0] << 5) | reg;
        emit(movz);
        std::cout << "movz x" << (int)reg << ", 0x" << std::hex << imm16[0]
                  << ", lsl #0 -> " << std::hex << movz << std::dec << std::endl;

        // MOVK Xd, imm16, LSL #16
        uint32_t movk16 = 0xF2800000 | (imm16[1] << 5) | reg | (1 << 21);
        emit(movk16);
        std::cout << "movk x" << (int)reg << ", 0x" << std::hex << imm16[1]
                  << ", lsl #16 -> " << std::hex << movk16 << std::dec << std::endl;

        // MOVK Xd, imm16, LSL #32
        uint32_t movk32 = 0xF2800000 | (imm16[2] << 5) | reg | (2 << 21);
        emit(movk32);
        std::cout << "movk x" << (int)reg << ", 0x" << std::hex << imm16[2]
                  << ", lsl #32 -> " << std::hex << movk32 << std::dec << std::endl;

        // MOVK Xd, imm16, LSL #48
        uint32_t movk48 = 0xF2800000 | (imm16[3] << 5) | reg | (3 << 21);
        emit(movk48);
        std::cout << "movk x" << (int)reg << ", 0x" << std::hex << imm16[3]
                  << ", lsl #48 -> " << std::hex << movk48 << std::dec << std::endl;
        
        std::cout << "mov_abs x" << (int)reg << ", 0x"
                          << std::hex << value << std::dec << std::endl;

    }

    inline uint8_t getTag(uint64_t raw) {
        return raw & 0b111;
    }

    inline uint64_t getPayload(uint64_t raw) {
        return raw >> 3;
    }

    inline uint64_t makeValue(uint64_t payload, uint8_t tag) {
        return (payload << 3) | (tag & 0b111);
    }

    uint8_t getTag(ValueTag v) {
        return v.raw & 0b111;
    }

    uint64_t getPayload(ValueTag v) {
        return v.raw >> 3;
    }

    void alignDataSection() {
        while (dataSection.size() % 8 != 0)
            dataSection.push_back(0);
    }

    size_t addStringData(const std::string& s) {
        alignDataSection();

        size_t addr = dataSection.size();

        // Write characters
        dataSection.insert(dataSection.end(), s.begin(), s.end());

        // Null terminator
        dataSection.push_back('\0');

        return addr;  // this is the string pointer
    }

    ValueTag makeStringValue(size_t addr) {
        ValueTag v;
        v.raw = ((uint64_t)addr << 3) | TAG_STRING;  // pointer tagging
        return v;
    }
    
    ValueTag addString(const std::string& s) {
        size_t addr = addStringData(s);
        return makeStringValue(addr);
    }

    ValueTag makeIntValue(int64_t i) {
        ValueTag v;
        v.raw = ((uint64_t)i << 3) | TAG_NUMBER;
        return v;
    }
    
    ValueTag makeBool(int64_t i) {
        ValueTag v;
        v.raw = ((uint64_t)i << 3) | TAG_BOOLEAN;
        return v;
    }
    
    ValueTag makeUndefined() {
        ValueTag v;
        v.raw = ((uint64_t)0 << 3) | TAG_UNDEFINED;
        return v;
    }

    const char* getStringPtr(ValueTag v) {
        return (const char*)&dataSection[getPayload(v)];
    }
    
    size_t allocSpace() {
        alignDataSection();
        size_t addr = dataSection.size();

        uint64_t v64 = (uint64_t)0;  // promote int → 8-byte integer

        // Write out all 8 bytes
        uint8_t* p = reinterpret_cast<uint8_t*>(&v64);
        dataSection.insert(dataSection.end(), p, p + sizeof(uint64_t));

        return addr;
    }
    
    size_t addValue(const ValueTag& v) {
        size_t addr = dataSection.size();
        uint64_t raw = v.raw;

        uint8_t* p = reinterpret_cast<uint8_t*>(&raw);
        dataSection.insert(dataSection.end(), p, p + sizeof(uint64_t));

        return addr;
    }

    size_t addData(const std::string& value) {
        size_t addr = (uint64_t)(dataSection.size());
        dataSection.insert(dataSection.end(), value.begin(), value.end());
        dataSection.push_back('\0');
        return addr;
    }
        
    size_t addNumericData(const int value) {
        alignDataSection();
        size_t addr = dataSection.size();

        uint64_t v64 = (uint64_t)value;  // promote int → 8-byte integer

        // Write out all 8 bytes
        uint8_t* p = reinterpret_cast<uint8_t*>(&v64);
        dataSection.insert(dataSection.end(), p, p + sizeof(uint64_t));

        return addr;
    }

    void b_eq(int label) {
        pendingBranches.push_back({static_cast<int>(code.size()), label, CondEQ});
        emit(0x54000000); // BEQ placeholder, patched in resolveLabels
        cout << "beq " << (int)label << '\n';
    }

    void bne(int label) {
        pendingBranches.push_back({static_cast<int>(code.size()), label, CondNE});
        emit(0x54000001); // BNE placeholder (BEQ | 1)
        cout << "bne " << (int)label << '\n';
    }

    void blt(int label) {
        pendingBranches.push_back({static_cast<int>(code.size()), label, CondLT});
        emit(0x5400000B); // BLT placeholder (BEQ | 0xB)
    }

    void ble(int label) {
        pendingBranches.push_back({static_cast<int>(code.size()), label, CondLE});
        emit(0x5400000D); // BLE placeholder (BEQ | 0xD)
    }

    void bgt(int label) {
        pendingBranches.push_back({static_cast<int>(code.size()), label, CondGT});
        emit(0x5400000C); // BGT placeholder (BEQ | 0xC)
    }

    void bge(int label) {
        pendingBranches.push_back({static_cast<int>(code.size()), label, CondGE});
        emit(0x5400000A); // BGE placeholder (BEQ | 0xA)
    }

    void b(int label) {
        pendingBranches.push_back({static_cast<int>(code.size()), label, CondAL});
        emit(0x14000000); // B placeholder, patched in resolveLabels
        cout << "b " << (int)label << '\n';
    }

    int genLabel() {
        return labelCounter++;
    }

    void setLabel(int label) {
        labels[label] = static_cast<int>(code.size());
        cout << "label: " << (int)label << '\n';
    }

    // Patch branch offsets after full emission
    void resolveLabels_() {
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

    void resolveLabels() {
        for (const auto& br : pendingBranches) {
            int src = br.pos;// / 4;
            int dst = labels[br.label];
            int offset = (dst - src);
            uint32_t* inst = reinterpret_cast<uint32_t*>(&code[br.pos]);
            switch (br.cond) {
                case CondAL:
                    // B (unconditional)
                    // *inst = 0x14000000 | (offset & 0x03FFFFFF);
                    *inst = 0x14000000 | (offset / 4);
                    break;
                case CondEQ:
                    // BEQ
                    // *inst = 0x54000000 | ((offset & 0x7FFFF) << 5);
                    *inst = 0x54000000 | ((offset / 4) << 5);
                    break;
                case CondNE:
                    // BNE
                    // *inst = 0x54000001 | ((offset & 0x7FFFF) << 5);
                    *inst = 0x54000001 | ((offset / 4) << 5);
                    break;
                case CondLT:
                    // BLT
                    *inst = 0x5400000B | ((offset & 0x7FFFF) << 5);
                    break;
                case CondLE:
                    // BLE
                    *inst = 0x5400000D | ((offset & 0x7FFFF) << 5);
                    break;
                case CondGT:
                    // BGT
                    *inst = 0x5400000C | ((offset & 0x7FFFF) << 5);
                    break;
                case CondGE:
                    // BGE
                    *inst = 0x5400000A | ((offset & 0x7FFFF) << 5);
                    break;
                default:
                    // Unknown condition
                    break;
            }
        }
    }
    
    void defineLabel(const std::string& name) {
        auto& lbl = _labels[name];
        lbl.name = name;
        lbl.offset = code.size() * 4;
        lbl.defined = true;
    }
    
    void emitBranchToLabel(const std::string& label, bool isCall) {
        size_t pos = code.size() * 4;

        uint32_t placeholder = isCall ? 0x94000000 : 0x14000000; // BL or B base opcode
        code.push_back(placeholder);

        relocations.push_back({ pos, label, isCall });
    }

    void patchRelocations(uint64_t baseAddress) {
        for (auto& reloc : relocations) {
            auto it = _labels.find(reloc.targetLabel);
            if (it == _labels.end() || !it->second.defined) {
                fprintf(stderr, "Undefined label: %s\n", reloc.targetLabel.c_str());
                continue;
            }

            uint64_t sourceAddr = baseAddress + reloc.sourceOffset;
            uint64_t targetAddr = baseAddress + it->second.offset;

            int64_t offset = (int64_t(targetAddr) - int64_t(sourceAddr)) >> 2;

            uint32_t opcode = reloc.isCall ? 0x94000000 : 0x14000000;
            uint32_t patched = opcode | (offset & 0x03FFFFFF);

            // Patch instruction in textSection
            size_t instrIndex = reloc.sourceOffset / 4;
            code[instrIndex] = patched;
        }
    }

    // Access raw code/data for output or execution
    const std::vector<uint8_t>& getCode() const { return code; }
    const std::vector<uint8_t>& getDataSection() const { return dataSection; }

private:
    
    struct Label {
        string name;
        size_t offset;   // offset in textSection (in bytes)
        bool defined = false;
    };

    struct Relocation {
        size_t sourceOffset;     // where the branch instruction is
        string targetLabel; // the name of the label it jumps to
        bool isCall;             // true = BL, false = B
    };

    enum Cond {
        CondAL,  // Always (unconditional)
        CondEQ,  // Equal (Z==1)
        CondNE,  // Not equal (Z==0)
        CondLT,  // Less than (N != V)
        CondLE,  // Less than or equal (Z==1 || N != V)
        CondGT,  // Greater than (Z==0 && N == V)
        CondGE,  // Greater than or equal (N == V)
    };
    
    struct Branch {
        int pos;
        int label;
        Cond cond;
    };

    int labelCounter;
    std::unordered_map<int, int> labels;
    std::vector<Branch> pendingBranches;
    std::unordered_map<std::string, Label> _labels;
    std::vector<Relocation> relocations;

    std::vector<uint8_t> code;
    std::vector<uint8_t> dataSection;
    
};

#endif /* ARM64Emitter_hpp */

